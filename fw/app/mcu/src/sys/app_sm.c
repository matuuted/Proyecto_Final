/**
 * @file    dispenser_sm.c
 * @brief   Main state machine implementation for the smart cat food dispenser.
 * @details
 *  Manages the complete dispenser operation cycle using FreeRTOS:
 *
 *  - ST_INIT:       Initializes the DS3231 RTC and actuator GPIO. Retries up
 *                   to BOOT_MAX_TRIES times before transitioning to ST_ERROR.
 *
 *  - ST_IDLE:       Blocks on the event queue with a 1 s timeout. On timeout,
 *                   reads the RTC and checks scheduled feeding slots. On event
 *                   (EVT_BUTTON or EVT_SCHEDULED), transitions to ST_DISPENSING.
 *
 *  - ST_DISPENSING: Activates the actuator for DISPENSER_ACTIVE_MS, then
 *                   deactivates it and returns to ST_IDLE. Ignores any events
 *                   received while dispensing (no re-entry).
 *
 *  - ST_ERROR:      Solid LED on. Triggers a system reset after ERROR_RESET_MS.
 *
 *  LED patterns (non-blocking, accumulator-based):
 *    ST_IDLE       → 1 Hz blink  (500 ms half-period)
 *    ST_DISPENSING → 5 Hz blink  (100 ms half-period)
 *    ST_ERROR      → solid on
 *
 * @author   Matías Durante
 * @version  1.0
 * @date     2025
 */

/* ========================================================================== */
/*                                  INCLUDES                                  */
/* ========================================================================== */
#include "app_sm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os.h"
#include "ds3231.h"
#include "dev_gpio_cfg.h"
#include "dev_uart.h"

#include <string.h>
#include <stdbool.h>

/* ========================================================================== */
/*                            CONFIGURATION PARAMETERS                        */
/* ========================================================================== */

/** @brief Maximum initialization attempts per peripheral. */
#ifndef BOOT_MAX_TRIES
#define BOOT_MAX_TRIES          3
#endif

/** @brief RTC time to set on first successful initialization. */
#define RTC_YEAR   25 
#define RTC_MONTH   4  
#define RTC_DATE   12  
#define RTC_DAY     1  
#define RTC_HOUR    8  
#define RTC_MIN     0  
#define RTC_SEC     40 

/** @brief Delay between initialization retries (ms). */
#define BOOT_RETRY_DELAY_MS     pdMS_TO_TICKS(200)

/** @brief Delay before system reset on ST_ERROR (ms). */
#define ERROR_RESET_MS          pdMS_TO_TICKS(10000)

/** @brief Queue receive timeout — defines the RTC polling interval (ms). */
#define IDLE_POLL_MS            pdMS_TO_TICKS(1000)

/** @brief Main task loop period (ms). */
#define TASK_PERIOD_MS          pdMS_TO_TICKS(100)

/** @brief LED blink half-period in ST_IDLE (ms). */
#define LED_IDLE_HALF_MS        50

/** @brief LED blink half-period in ST_DISPENSING (ms). */
#define LED_DISPENSING_HALF_MS  100

/* ========================================================================== */
/*                            SCHEDULE CONFIGURATION                          */
/* ========================================================================== */

/**
 * @brief Hardcoded feeding schedule (up to DISPENSER_MAX_SLOTS entries).
 * @note  Modify hours/minutes here to change feeding times.
 *        Set slot count in SCHEDULE_COUNT accordingly.
 */
static const Dispenser_Time s_schedule[DISPENSER_MAX_SLOTS] = {
    { .hour =  8, .minute =  1 },  /**< Morning feed   08:00 */
    { .hour = 13, .minute =  0 },  /**< Midday feed    13:00 */
    { .hour = 18, .minute =  0 },  /**< Evening feed   18:00 */
    { .hour = 22, .minute =  0 },  /**< Night feed     22:00 */
};

/** @brief Number of active slots in s_schedule. */
#define SCHEDULE_COUNT  4

/* ========================================================================== */
/*                          TYPES AND LOCAL STRUCTURES                        */
/* ========================================================================== */

/**
 * @brief Peripheral initialization stages during boot sequence.
 */
typedef enum {
    DEV_INIT_RTC      = 0,
    DEV_INIT_COMPLETE = 1,
    DEV_INIT_ERROR    = 2,
} Device_Init_Stage;

/**
 * @brief Boot sequence tracking structure.
 */
typedef struct {
    Device_Init_Stage stage;         /**< Current initialization stage.       */
    uint8_t           attempt_count; /**< Retry counter for current stage.    */
    bool              rtc_ready;     /**< RTC initialized successfully.       */
} Boot_Status;

/**
 * @brief Main state machine handler.
 *        Holds all runtime state for the dispenser SM.
 */
typedef struct {
    SM_State     state;             /**< Current SM state.                      */
    DS3231_Time  rtc;               /**< Last RTC reading.                      */
    uint32_t     led_acc_ms;        /**< LED blink accumulator (ms).            */
    bool         last_slot_fired;   /**< Guards against re-firing same minute.  */
    uint8_t      last_fired_hour;   /**< Hour of last fired slot.               */
    uint8_t      last_fired_min;    /**< Minute of last fired slot.             */
    TickType_t   dispense_start;    /**< Timestamp when dispensing started.     */
    bool         dispensing_active; /**< Entry action guard                     */
    SM_Event    dispense_source;    /**< Source of current dispense event.      */
} SM_Handler;

/* ========================================================================== */
/*                               LOCAL VARIABLES                              */
/* ========================================================================== */

static SM_Handler   s_sm;
static Boot_Status  s_boot;
static QueueHandle_t s_event_queue;

/* ========================================================================== */
/*                            PRIVATE FUNCTIONS                               */
/* ========================================================================== */

/**
 * @brief  Checks whether the current RTC time matches any scheduled slot.
 * @param  t     Pointer to the current RTC time.
 * @return true if a slot matches and has not already been fired this minute.
 */
static bool schedule_check(const DS3231_Time *t)
{
    if (!t) return false;

    for (uint8_t i = 0; i < SCHEDULE_COUNT; i++)
    {
        if (t->hours == s_schedule[i].hour && t->minutes == s_schedule[i].minute && t->seconds < 5U)
        {
            if (s_sm.last_slot_fired && s_sm.last_fired_hour == t->hours && s_sm.last_fired_min  == t->minutes)
            {
                return false;
            }

            s_sm.last_slot_fired  = true;
            s_sm.last_fired_hour  = t->hours;
            s_sm.last_fired_min   = t->minutes;
            return true;
        }
    }

    /* Reset the guard when we leave the matched minute */
    if (s_sm.last_slot_fired)
    {
        bool still_in_slot = false;
        for (uint8_t i = 0; i < SCHEDULE_COUNT; i++)
        {
            if (t->hours == s_schedule[i].hour &&
                t->minutes == s_schedule[i].minute)
            {
                still_in_slot = true;
                break;
            }
        }
        if (!still_in_slot) s_sm.last_slot_fired = false;
    }

    return false;
}

/**
 * @brief  Updates the status LED blink pattern based on the current SM state.
 * @note   Called every TASK_PERIOD_MS. Uses a non-blocking accumulator.
 * @param  elapsed_ms  Time elapsed since last call (ms).
 */
static void led_update(uint32_t elapsed_ms)
{
    switch (s_sm.state)
    {
        case ST_IDLE:
            s_sm.led_acc_ms += elapsed_ms;
            if (s_sm.led_acc_ms >= LED_IDLE_HALF_MS)
            {
                HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
                s_sm.led_acc_ms = 0;
            }
            break;

        case ST_DISPENSING:
            s_sm.led_acc_ms += elapsed_ms;
            if (s_sm.led_acc_ms >= LED_DISPENSING_HALF_MS)
            {
                HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
                s_sm.led_acc_ms = 0;
            }
            break;

        case ST_ERROR:
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
            break;

        default:
            break;
    }
}

/**
 * @brief  Sends a dispense event log line over UART with RTC timestamp.
 * @param  t       Pointer to the current RTC time.
 * @param  source  EVT_BUTTON for manual, EVT_SCHEDULED for automatic.
 */
static void log_dispense_event(const DS3231_Time *t, SM_Event source)
{
    if (!t) return;

    uint8_t buf[UART_TX_MAX_SIZE];
    const char *src_str = (source == EVT_BUTTON) ? "MANUAL" : "SCHEDULED";

    int len = snprintf((char *)buf, sizeof(buf),
                       "[%02u:%02u:%02u] DISPENSE %s — active for %u ms\r\n",
                       t->hours, t->minutes, t->seconds,
                       src_str, DISPENSER_ACTIVE_MS);

    if (len > 0) uartSendString(buf);
}

/**
 * @brief  Runs one step of the peripheral initialization sequence.
 * @return Current Device_Init_Stage after this step.
 */
static Device_Init_Stage run_device_initialization(void)
{
    switch (s_boot.stage)
    {
        case DEV_INIT_RTC:
            if (!s_boot.rtc_ready)
            {
                s_boot.rtc_ready = (DS3231_Init() == DS3231_OK);
                if (s_boot.rtc_ready)
                {
                    if (DS3231_SetTime(RTC_YEAR, RTC_MONTH, RTC_DATE, RTC_DAY,  RTC_HOUR,  RTC_MIN, RTC_SEC) != DS3231_OK)
                    {
                        return DEV_INIT_ERROR;
                    }
                    s_boot.attempt_count = 0;
                    s_boot.stage = DEV_INIT_COMPLETE;
                    return DEV_INIT_COMPLETE;
                }
                if (++s_boot.attempt_count >= BOOT_MAX_TRIES)
                    return DEV_INIT_ERROR;

                osDelay(BOOT_RETRY_DELAY_MS);
                return DEV_INIT_RTC;
            }
            return DEV_INIT_COMPLETE;

        default:
            return DEV_INIT_ERROR;
    }
}

/* ========================================================================== */
/*                              PUBLIC FUNCTIONS                               */
/* ========================================================================== */

HAL_StatusTypeDef SM_InitOS(void)
{
    memset(&s_sm,   0, sizeof(s_sm));
    memset(&s_boot, 0, sizeof(s_boot));

    s_sm.state   = ST_INIT;
    s_boot.stage = DEV_INIT_RTC;


    HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_GPIO_Pin, GPIO_PIN_RESET);

    /* Create the event queue — depth 4, each item is one SM_Event */
    s_event_queue = xQueueCreate(4, sizeof(SM_Event));
    if (s_event_queue == NULL) return HAL_ERROR;

    static const osThreadAttr_t task_attr = {
        .name       = "SMTask",
        .stack_size = 512 * 4,
        .priority   = (osPriority_t) osPriorityNormal,
    };

    osThreadId_t handle = osThreadNew(SM_Task, NULL, &task_attr);
    return (handle == NULL) ? HAL_ERROR : HAL_OK;
}

void Dispenser_PostButtonEvent(void)
{
    SM_Event evt = EVT_BUTTON;
    BaseType_t higher_priority_task_woken = pdFALSE;

    xQueueSendFromISR(s_event_queue, &evt, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void SM_Task(void *argument)
{
    (void)argument;

    TickType_t last_wake = xTaskGetTickCount();

    for (;;)
    {
        switch (s_sm.state)
        {
            /* ----------------------------------------------------------------
             * ST_INIT — initialize peripherals before entering the main loop
             * -------------------------------------------------------------- */
            case ST_INIT:
            {
                Device_Init_Stage stage = run_device_initialization();

                if (stage == DEV_INIT_COMPLETE)
                {
                    s_sm.state = ST_IDLE;
                    break;
                }

                if (stage == DEV_INIT_ERROR)
                {
                    s_sm.state = ST_ERROR;
                    break;
                }

                osDelay(50);
            }
            break;

            /* ----------------------------------------------------------------
             * ST_IDLE — wait for events; poll RTC on timeout
             * -------------------------------------------------------------- */
            case ST_IDLE:
            {
                SM_Event evt = EVT_NONE;

                if (xQueueReceive(s_event_queue, &evt, IDLE_POLL_MS) == pdTRUE)
                {
                    /* Manual button press — go dispense immediately */
                    if (evt == EVT_BUTTON)
                    {
                        s_sm.dispense_source = EVT_BUTTON;
                        s_sm.state      = ST_DISPENSING;
                        s_sm.led_acc_ms = 0;
                        break;
                    }
                }

                /* Queue timeout: check RTC against schedule */
                if (DS3231_ReadTime(&s_sm.rtc) == DS3231_OK)
                {
                    if (schedule_check(&s_sm.rtc))
                    {
                        s_sm.dispense_source = EVT_SCHEDULED;
                        s_sm.state           = ST_DISPENSING;
                        s_sm.led_acc_ms      = 0;
                    }
                }
            }
            break;

            /* ----------------------------------------------------------------
             * ST_DISPENSING — activate actuator for DISPENSER_ACTIVE_MS
             * -------------------------------------------------------------- */
            case ST_DISPENSING:
            {
                if (!s_sm.dispensing_active)
                {
                    log_dispense_event(&s_sm.rtc, s_sm.dispense_source);
                    HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_GPIO_Pin, GPIO_PIN_SET);
                    s_sm.dispense_start = xTaskGetTickCount();
                    s_sm.dispensing_active = true;
                }

                /* Check if dispense duration has elapsed */
                TickType_t elapsed = xTaskGetTickCount() - s_sm.dispense_start;
                if (elapsed >= pdMS_TO_TICKS(DISPENSER_ACTIVE_MS))
                {
                    HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_GPIO_Pin, GPIO_PIN_RESET);
                    s_sm.dispensing_active = false;
                    s_sm.state             = ST_IDLE;
                    s_sm.led_acc_ms        = 0;
                    xQueueReset(s_event_queue);
                }
            }
            break;

            /* ----------------------------------------------------------------
             * ST_ERROR — solid LED on, reset after ERROR_RESET_MS
             * -------------------------------------------------------------- */
            case ST_ERROR:
            {
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
                osDelay(ERROR_RESET_MS);
                NVIC_SystemReset();
            }
            break;

            default:
                s_sm.state = ST_INIT;
            break;
        }

        /* --------------------------------------------------------------------
         * LED update — runs every TASK_PERIOD_MS regardless of state
         * (except ST_DISPENSING which uses vTaskDelay internally)
         * ------------------------------------------------------------------ */
        if (s_sm.state != ST_ERROR)
        {
            led_update(100U);
        }

        vTaskDelayUntil(&last_wake, TASK_PERIOD_MS);
    }
}
