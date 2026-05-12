/**
 * @file    dev_hx711.c
 * @brief   Driver para dos módulos HX711 (tanque + plato).
 */

#include "dev_hx711.h"
#include "hx711_registers.h"
#include "cmsis_os2.h"
#include "dev_uart.h"

/* =========================================================================== */
/*                          ESTADO INTERNO POR CANAL                          */
/* =========================================================================== */

typedef struct {
    int32_t offset;
    float   scale;
} HX711_State;

static HX711_State s_state[HX711_CH_COUNT] = {
    { .offset = 0, .scale = HX711_SCALE_TANK_DEFAULT  },
    { .offset = 0, .scale = HX711_SCALE_PLATE_DEFAULT },
};

/** @brief Mutex — protege acceso simultáneo desde SM_Task y CommTask. */
static osMutexId_t s_hx711_mutex = NULL;

/* =========================================================================== */
/*                          FUNCIONES PRIVADAS                                 */
/* =========================================================================== */

/**
 * @brief  Convierte 24 bits unsigned a int32_t (complemento a dos).
 */
static int32_t to_signed(uint32_t raw)
{
    if (raw & 0x800000U)
        raw |= 0xFF000000U;  /* Extensión de signo */
    return (int32_t)raw;
}

/**
 * @brief  Lee HX711_AVG_SAMPLES muestras y retorna el promedio.
 */
static HX711_Status read_average(uint8_t ch, int32_t *avg)
{
    int64_t sum = 0;

    for (uint8_t i = 0; i < HX711_AVG_SAMPLES; i++) {
        if (!HX711_port_wait_ready(ch)) return HX711_TIMEOUT;
        sum += to_signed(HX711_port_read_raw(ch));
    }

    *avg = (int32_t)(sum / (int64_t)HX711_AVG_SAMPLES);
    return HX711_OK;
}

/* =========================================================================== */
/*                          API PÚBLICA                                        */
/* =========================================================================== */

HX711_Status HX711_Init(void)
{
    /* Crear mutex antes de arrancar el RTOS — se llama desde devices_Init() */
    s_hx711_mutex = osMutexNew(NULL);
    /* Nota: osMutexNew puede retornar NULL si el kernel no arrancó todavía.
     * En ese caso el mutex se crea en HX711_TareAll/ReadGrams si es NULL. */

    HX711_port_init();

    /* Despertar ambos HX711 */
    for (uint8_t ch = 0; ch < HX711_CH_COUNT; ch++) {
        if (!HX711_port_wait_ready(ch)) return HX711_ERROR;
        (void)HX711_port_read_raw(ch);
    }

    return HX711_OK;
}

/**
 * @brief  Crea el mutex si todavía no existe — llamar desde una tarea RTOS.
 */
static void ensure_mutex(void)
{
    if (s_hx711_mutex == NULL)
        s_hx711_mutex = osMutexNew(NULL);
}

HX711_Status HX711_ReadGrams(uint8_t ch, float *grams)
{
    if (ch >= HX711_CH_COUNT || grams == NULL) return HX711_ERROR;

    ensure_mutex();
    osMutexAcquire(s_hx711_mutex, osWaitForever);

    int32_t avg;
    HX711_Status st = read_average(ch, &avg);

    osMutexRelease(s_hx711_mutex);
    if (st != HX711_OK) return st;

    float result = (float)(avg - s_state[ch].offset) / s_state[ch].scale;
    *grams = (result < -5.0f) ? 0.0f : result;

    return HX711_OK;
}

HX711_Status HX711_Tare(uint8_t ch)
{
    if (ch >= HX711_CH_COUNT) return HX711_ERROR;

    ensure_mutex();
    osMutexAcquire(s_hx711_mutex, osWaitForever);

    int32_t avg;
    HX711_Status st = read_average(ch, &avg);

    if (st == HX711_OK) {
        s_state[ch].offset = avg;

        /* Debug — mostrar offset guardado */
        uint8_t buf[64];
        snprintf((char *)buf, sizeof(buf),
                 "[HX711] CH%d tara OK, offset=%ld\r\n", ch, (long)avg);
        uartSendString(buf);
    }

    osMutexRelease(s_hx711_mutex);
    return st;
}

HX711_Status HX711_TareAll(void)
{
    HX711_Status st;
    st = HX711_Tare(HX711_CH_TANK);
    if (st != HX711_OK) return st;
    return HX711_Tare(HX711_CH_PLATE);
}

void HX711_SetScale(uint8_t ch, float scale)
{
    if (ch >= HX711_CH_COUNT || scale == 0.0f) return;
    s_state[ch].scale = scale;
}

void HX711_PowerDown(void)
{
    for (uint8_t ch = 0; ch < HX711_CH_COUNT; ch++)
        HX711_port_power_down(ch);
}

void HX711_PowerUp(void)
{
    for (uint8_t ch = 0; ch < HX711_CH_COUNT; ch++)
        HX711_port_power_up(ch);
    osDelay(500); /* Esperar a que el HX711 se estabilice después de despertar */
}