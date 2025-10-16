/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sys_sm.c
  * @brief   Máquina de estados principal del sistema.
  *
  * Esta SM se encarga de coordinar todo el flujo del dispositivo: arranca los 
  * periféricos (LCD, RTC, MPU6050), realiza las lecturas periódicas y muestra 
  * la información tanto en pantalla como por UART. Además, controla el parpadeo 
  * del LED según el ángulo de inclinación detectado.
  *
  * Ciclo principal (≈100 ms):
  *   1. Lee los sensores.
  *   2. Calcula el ángulo de inclinación.
  *   3. Ajusta la frecuencia del LED según el ángulo.
  *   4. Actualiza la hora y el ángulo en LCD y UART.
  *
  * Flujo de estados:
  *   ST_INIT 
  *     ↓
  *   ST_READ_SENSORS → ST_UPDATE_LED → ST_UPDATE_UART → (vuelve a leer)
  *
  * @author   Matías Durante
  * @version  1.0
  * @date     2025
  ******************************************************************************
  */
/* USER CODE END Header */

/* ========================================================================== */
/*                                   INCLUDES                                 */
/* ========================================================================== */
#include "sys_sm.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "dev_LCD.h"
#include "mpu6050.h"
#include "ds3231.h"
#include "mpu_math.h"
#include "dev_gpio_cfg.h"
#include "dev_uart.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* ========================================================================== */
/*                     PARAMETROS DE MUESTREO                                 */
/* ========================================================================== */
/**
 * @brief Configuración del ciclo de muestreo y actualización del sistema.
 * 
 * - El loop principal corre cada 100 ms (PERIOD_100MS).
 * - Se lee la información del RTC y se actualizan los valores del LCD cada 1 segundo.
 */
#define SAMPLE_PERIOD_MS        100
#define PERIOD_100MS            pdMS_TO_TICKS(100) 
#define PERIOD_1S               pdMS_TO_TICKS(1000)
#define UART_REFRESH_DIV        5   
#define ALPHA                   0.2f                /* Filtro utilizado para el calculo del angulo*/
#define STR_LENGTH              DEV_LCD_COLS + 1    /* 16 Caracteres del LCD + \0 */
#define UART_STR_LENGTH         48                  /* 48 Caracteres enviados por la UART */

#ifndef BOOT_MAX_TRIES        
#define BOOT_MAX_TRIES 3                            /* Numero de intentos de inicialización por periférico */
#endif

/* ========================================================================== */
/*                PARAMETROS DE CONFIGURACION DEL RTC                         */
/* ========================================================================== */
#define RTC_YEAR   25
#define RTC_MONTH  10
#define RTC_DATE   15
#define RTC_DAY    3
#define RTC_HOUR   14
#define RTC_MIN    30
#define RTC_SEC    0

/* ========================================================================== */
/*                          TIPOS Y ESTRUCTURAS LOCALES                       */
/* ========================================================================== */

/**
 * @brief Estado de inicialización de periféricos durante el arranque del sistema.
 */
typedef enum {
    DEV_INIT_LCD = 0,
    DEV_INIT_RTC,
    DEV_INIT_MPU,
    DEV_INIT_COMPLETE,
    DEV_INIT_ERROR
} Device_Init_Stage;


/**
 * @brief  Estructura de control que contiene todas las variables de la SM principal.
 * 
 * Esta estructura conserva el estado actual del sistema, los datos de sensores,
 * y los contadores asociados al ciclo de muestreo.
 */
typedef struct {
    SM_State        state;     /*  Estado actual de la máquina de estados */
    MPU6050_Handler mpu;       /*  Handler del  MPU6050 */
    DS3231_Time     rtc;       /*  Estructura de tiempo del DS3231 */

    float        angle;        /* Ángulo filtrado actual */
    TickType_t   blink_period; /* Periodo del LED en ticks */
    TickType_t   blink_acc;    /* Acumulador en ticks */
} SM_Handler;

/**
 * @brief Estructura utilizada en el proceso de inicialización de dispositivos.
 * 
 * Permite reintentar los periféricos que fallaron, evitando reinicios innecesarios de los dispositivos ya inicializados.
 */
typedef struct {
    Device_Init_Stage init_stage;
    uint8_t   attempt_count;
    bool      lcd_ready;
    bool      rtc_ready;
    bool      mpu_ready;
} Boot_Status;


/* ========================================================================== */
/*                               VARIABLES LOCALES                            */
/* ========================================================================== */
static Boot_Status boot_status;
static SM_Handler sm;


/* ========================================================================== */
/*                             FUNCIONES AUXILIARES                           */
/* ========================================================================== */

/**
 * @brief Convierte un ángulo en milisegundos de parpadeo LED.
 *
 * | Ángulo | Frecuencia | Período |
 * |--------|-------------|----------|
 * | 0–10°  | 1 Hz        | 1000 ms  |
 * | 10–25° | 2 Hz        | 250 ms   |
 * | 25–40° | 4 Hz        | 125 ms   |
 * | >40°   | 8 Hz        | 50 ms    |
*/

static uint32_t angle_to_period_ms(float angle_deg)
{
    float angle = fabsf(angle_deg);

    if      (angle < 10.0) return 1000;
    else if (angle < 25.0) return 250;
    else if (angle < 40.0) return 125;
    else                    return 50;
}


/**
 * @brief Convierte un ángulo float a entero.
 * 
 */
static inline int angle_to_int(float angle_deg)
{
    if (angle_deg != angle_deg) angle_deg = 0.0;        /*  Si el valor es NaN, se reemplaza por  -> 0 */

    int angle = (angle_deg >= 0.0) ? (int)(angle_deg + 0.5) : (int)(angle_deg - 0.5);

    /* Limito el angulo a +- 90 grados para evitar problemas de lecturas. */
    if (angle >  90) angle =  90;
    if (angle < -90) angle = -90;
    return angle;
}

/**
 * @brief Genera un formato de texto con hora e inclinación actual.
 * 
 */

static void format_text(const DS3231_Time *t, float angle_deg, char *line0, char *line1)
{
    int angle = angle_to_int(angle_deg);
    snprintf(line0, STR_LENGTH, "Hora:%02u:%02u:%02u", t->hours, t->minutes, t->seconds);  /* Línea 0: "Hora:HH:MM:SS" */
    snprintf(line1, STR_LENGTH, "Incl:%3d%c", angle, (char)0xDF);                          /* Línea 1: "Incl: %3d°"  -> 0xDF = símbolo ° */
}

/**
 * @brief Enviar por UART una línea de texto formateada con información de tiempo y un ángulo en grados.
 * 
 */

static void uart_send_data(const DS3231_Time *t, float angle_deg)
{
    char line0[STR_LENGTH], line1[STR_LENGTH], out[UART_STR_LENGTH];
    format_text(t, angle_deg, line0, line1);
    int n = snprintf(out, sizeof(out), "%s | %s\r\n", line0, line1);
    if (n > 0) {
        uartSendString((uint8_t*)out); 
    }
}

/* ========================================================================== */
/*                               FUNCIONES LOCALES                            */
/* ========================================================================== */

/**
 * @brief Muestra un mensaje en el LCD durante el proceso de arranque.
 * 
 * Permite informar visualmente el estado de cada periférico durante la etapa de inicialización de los dispositivos.
 */
static void show_boot_message(const char* line1, const char* line2)
{
    if (!boot_status.lcd_ready) return;

    LCD_Clear();
    LCD_PrintCentered(DEV_LCD_ROWS_0, (char*)line1);
    LCD_WriteAt(DEV_LCD_COLS_0, DEV_LCD_ROWS_1, (char*)line2);
}

/**
 * @brief Ejecuta la inicialización secuencial de los periféricos.
 * 
 * En cada llamada a la funcion, se intenta inicializar un solo dispositivo diferente (LCD → RTC → MPU).
 * Si la inicializacion de un periférico falla, se reintenta hasta un máximo de BOOT_MAX_TRIES veces antes de pasar al estado DEV_INIT_ERROR.
 * 
 * @return Device_Init_Stage Estado de inicializacion de los dispositivos.
 */
static Device_Init_Stage run_device_initialization(void)
{
    switch (boot_status.init_stage)
    {
        case DEV_INIT_LCD:
            /* LCD: Muestreo del estado de la Inicializacion de los Perifericos.
                 Se inicializa primero el LCD para informar el estado de los otros dispositivos.
            */
            if (!boot_status.lcd_ready) {
                boot_status.lcd_ready = LCD_Init();
                if (boot_status.lcd_ready) {
                    show_boot_message("LCD OK", " ");
                    boot_status.attempt_count = 0;
                    boot_status.init_stage = DEV_INIT_RTC;
                    osDelay(1000);
                    return boot_status.init_stage;
                }
                if (++boot_status.attempt_count >= BOOT_MAX_TRIES)
                    return DEV_INIT_ERROR;

                osDelay(200);
                return DEV_INIT_LCD;
            }
            boot_status.init_stage = DEV_INIT_RTC;
            return boot_status.init_stage;

        case DEV_INIT_RTC:
            /* RTC: Sincroniza el reloj DS3231 */
            if (!boot_status.rtc_ready) {
                boot_status.rtc_ready = (DS3231_Init() == DS3231_OK);
                if (boot_status.rtc_ready) {
                    show_boot_message("DS3231 OK", " ");
                    boot_status.attempt_count = 0;
                    DS3231_SetTime(RTC_YEAR, RTC_MONTH, RTC_DATE, RTC_DAY, RTC_HOUR, RTC_MIN, RTC_SEC);
                    boot_status.init_stage = DEV_INIT_MPU;
                    osDelay(1000);
                    return boot_status.init_stage;
                }
                if (++boot_status.attempt_count >= BOOT_MAX_TRIES) {
                    show_boot_message("Error DS3231", " ");
                    return DEV_INIT_ERROR;
                }
                osDelay(200);
                return DEV_INIT_RTC;
            }
            boot_status.init_stage = DEV_INIT_MPU;
            return boot_status.init_stage;

        case DEV_INIT_MPU:
            /* MPU6050: Sensor de aceleración e inclinación */
            if (!boot_status.mpu_ready) {
                boot_status.mpu_ready = (MPU6050_Init(&sm.mpu) == MPU6050_OK);
                if (boot_status.mpu_ready) {
                    show_boot_message("MPU6050 OK", " ");
                    boot_status.init_stage = DEV_INIT_COMPLETE;
                    osDelay(1000);
                    LCD_Clear();
                    return DEV_INIT_COMPLETE;
                }
                if (++boot_status.attempt_count >= BOOT_MAX_TRIES) {
                    show_boot_message("Error MPU6050", " ");
                    return DEV_INIT_ERROR;
                }
                osDelay(200);
                return DEV_INIT_MPU;
            }
            return (boot_status.init_stage = DEV_INIT_COMPLETE);

        default:
            return DEV_INIT_ERROR;
    }
}


/* ========================================================================== */
/*                              FUNCIONES DEL SISTEMA                         */
/* ========================================================================== */


HAL_StatusTypeDef SM_InitOS(void)
{
    /* Inicialización de estructuras */
    memset(&sm, 0, sizeof(sm));
    sm.state    = ST_INIT;
    sm.blink_period = PERIOD_1S;

    memset(&boot_status, 0, sizeof(boot_status));
    boot_status.init_stage = DEV_INIT_LCD;
    boot_status.attempt_count = 0;

    /* Creación de la tarea principal */
    static const osThreadAttr_t SMTask_attributes = {
        .name = "SMTask",
        .stack_size = 512 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };
    
    osThreadId_t SMTaskHandle = osThreadNew(SM_Iter, NULL, &SMTask_attributes);
    return (SMTaskHandle == NULL) ? HAL_ERROR : HAL_OK;
}

/**
 * @brief Máquina de estados principal del sistema.
 *
 * Estados:
 *  - ST_INIT: Inicializa periféricos. Si inicializan correctamente, se pasa al estado ST_READ_SENSORS; En caso de falla luego de los reintentos, se pasa al estado ST_ERROR.
 *  - ST_READ_SENSORS: Lee la información del MPU6050, filtra el ángulo y pasa a ST_UPDATE_LED.
 *  - ST_UPDATE_LED: Ajusta el periodo de parpadeo según el ángulo; pasa a ST_UPDATE_UART.
 *  - ST_UPDATE_UART: Envía hora y ángulo por UART; vuelve a ST_READ_SENSORS.
 *  - ST_ERROR: Muestra un mensaje de error en el LCD y reinicia el MCU.
 *
 *  - RTC: Actualiza el LCD cada 1 s con hora y ángulo.
 *  - LED: Parpadea según la inclinación calculada.
 */
void SM_Iter(void *argument)
{
    (void)argument;

    
    TickType_t last_wake = xTaskGetTickCount(); /* Tick del sistema, utilizado para mantener el timing preciso en el loop. */
    TickType_t last_rtc  = last_wake;           /* referencia para lectura 1 Hz del RTC. */

    for (;;)
    {
        switch (sm.state)
        {
            case ST_INIT:
            {
                Device_Init_Stage device_init = run_device_initialization();

                if (device_init == DEV_INIT_COMPLETE) {
                    show_boot_message("Iniciando...", "MPU+RTC listos");
                    osDelay(1000);
                    LCD_Clear();
                    sm.state = ST_READ_SENSORS;
                    break;
                }

                if (device_init == DEV_INIT_ERROR) {
                    show_boot_message("Boot fallido", "Abortando SM");
                    osDelay(1000);
                    sm.state = ST_ERROR;
                    break;
                }

                /* Todavía se encuentra inicializando los perifericos. Espera y vuelve a intentar */
                osDelay(50);
            }
            break;

            case ST_READ_SENSORS:
            {
                if (MPU6050_Read(&sm.mpu) == MPU6050_OK) {
                    float angle_deg = mpu_get_angle_deg(sm.mpu.data.accel_x, sm.mpu.data.accel_y, sm.mpu.data.accel_z);
                    sm.angle = (ALPHA * angle_deg) + (1.0f - ALPHA) * sm.angle;
                }
                sm.state = ST_UPDATE_LED;
            }
            break;

            case ST_UPDATE_LED:
            {
                sm.blink_period = angle_to_period_ms(sm.angle);
                sm.state = ST_UPDATE_UART;
            }
            break;

            case ST_UPDATE_UART:
            {
                uart_send_data(&sm.rtc, sm.angle);
                sm.state = ST_READ_SENSORS;
            }
            break;

            case ST_ERROR:
            {
                if (boot_status.lcd_ready) {
                    LCD_Clear();
                    LCD_PrintCentered(DEV_LCD_ROWS_0,"Error boot");
                    LCD_PrintCentered(DEV_LCD_ROWS_1,"Reiniciando...");
                }
                osDelay(3000);
                NVIC_SystemReset();
            }
            break;

            default:
                sm.state = ST_INIT;
            break;
        }

        /* =========================================================
         * Lectura del RTC y actualizacion del LCD cada 1s.
         * ========================================================= */
        if (sm.state != ST_INIT) { 
            TickType_t now = xTaskGetTickCount();
            if ((now - last_rtc) >= PERIOD_1S) {
                last_rtc += PERIOD_1S;
                if (DS3231_ReadTime(&sm.rtc) == DS3231_OK) {
                        char line0[STR_LENGTH], line1[STR_LENGTH];
                        format_text(&sm.rtc, sm.angle, line0, line1);
                        LCD_WriteAt(DEV_LCD_COLS_0, DEV_LCD_ROWS_0, line0);
                        LCD_WriteAt(DEV_LCD_COLS_0, DEV_LCD_ROWS_1, line1);
                }
            }
        }
        /* =========================================================
        * La frecuencia de parpadeo del LED se ajusta según el ángulo.
        * ========================================================= */
        if (sm.state != ST_INIT) { 
            sm.blink_acc += SAMPLE_PERIOD_MS;
            if (sm.blink_acc >= sm.blink_period) {
                HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
                sm.blink_acc = 0;
            }
        }
        vTaskDelayUntil(&last_wake, PERIOD_100MS);
    }
}
