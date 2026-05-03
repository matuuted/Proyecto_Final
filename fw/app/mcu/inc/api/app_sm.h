/**
 * @file    dispenser_sm.h
 * @brief   Main state machine for the smart cat food dispenser.
 * @details
 *  This module manages the complete dispenser operation cycle:
 *  initializes peripherals (RTC, actuator), monitors scheduled
 *  feeding times, handles manual dispense requests via button,
 *  and controls the status LED pattern for each system state.
 *
 *  State flow:
 *    ST_INIT
 *      ↓
 *    ST_IDLE ──── (scheduled time or button press) ────► ST_DISPENSING
 *      ▲                                                       │
 *      └───────────────────────────────────────────────────────┘
 *                        (dispense complete)
 *
 *  LED patterns:
 *    ST_IDLE       → slow blink (1 Hz)
 *    ST_DISPENSING → fast blink (5 Hz)
 *    ST_ERROR      → solid on
 *
 * @author   Matías Durante
 * @version  1.0
 * @date     2025
 */

#ifndef DISPENSER_SM_H
#define DISPENSER_SM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"
#include "ds3231.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* DISPENSE SCHEDULE                                                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief Maximum number of scheduled feeding times supported.
 */
#define DISPENSER_MAX_SLOTS     4U

/**
 * @brief Duration the actuator stays on during a single dispense cycle (ms).
 * @note  Adjust to calibrate the food ration amount.
 */
#define DISPENSER_ACTIVE_MS     3000U

/* -------------------------------------------------------------------------- */
/* ENUMS AND STRUCTS                                                          */
/* -------------------------------------------------------------------------- */

/**
 * @brief System states of the dispenser state machine.
 */
typedef enum {
    ST_INIT       = 0, /**< Peripheral initialization.              */
    ST_IDLE       = 1, /**< Waiting for a scheduled or manual event. */
    ST_DISPENSING = 2, /**< Actuator active, dispensing food.        */
    ST_ERROR      = 3, /**< Unrecoverable initialization failure.    */
} SM_State;

/**
 * @brief Event types posted to the dispenser event queue.
 */
typedef enum {
    EVT_NONE      = 0, /**< No event (queue timeout).               */
    EVT_BUTTON    = 1, /**< Manual dispense request from button ISR. */
    EVT_SCHEDULED = 2, /**< Scheduled dispense triggered by RTC.     */
} SM_Event;

/**
 * @brief A single scheduled feeding time (hours and minutes only).
 */
typedef struct {
    uint8_t hour;   /**< Hour of the feeding   (0–23). */
    uint8_t minute; /**< Minute of the feeding (0–59). */
} Dispenser_Time;

/* -------------------------------------------------------------------------- */
/* Public API                                                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Initializes the dispenser state machine and creates the FreeRTOS task.
 * @note   Must be called once from main(), before osKernelStart().
 * @return HAL_OK if the task was created successfully, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef SM_InitOS(void);

/**
 * @brief  Posts a manual dispense event to the queue from the button ISR.
 * @note   Safe to call from interrupt context (uses xQueueSendFromISR).
 */
void Dispenser_PostButtonEvent(void);

/**
 * @brief  Main state machine task. Called internally by FreeRTOS.
 * @param  argument  Unused (required by osThreadNew signature).
 */
void SM_Task(void *argument);

/** @example
 *  @code
 *  // In main(), before osKernelStart():
 *  if (Dispenser_InitOS() != HAL_OK) {
 *      Error_Handler();
 *  }
 *  osKernelStart();
 *  @endcode
 */

#ifdef __cplusplus
}
#endif

#endif /* DISPENSER_SM_H */
