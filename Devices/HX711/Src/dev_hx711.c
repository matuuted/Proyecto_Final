/**
 * @file    dev_hx711.c
 * @brief   Driver para dos módulos HX711 (tanque + plato).
 *
 * @details
 *  Protocolo HX711 Canal A Gain 128 (25 pulsos SCK):
 *   1. Esperar DOUT = LOW (dato listo, ~100 ms entre muestras a 10 Hz).
 *   2. 24 pulsos SCK → leer 1 bit DOUT por pulso (MSB first).
 *   3. 1 pulso extra (total 25) → Canal A Gain 128 para la próxima lectura.
 *   4. Valor de 24 bits en complemento a dos → convertir a int32_t.
 *   5. (crudo - offset) / scale = gramos.
 *
 * @author  Matías Durante
 * @version 1.0
 * @date    2025
 */

#include "dev_hx711.h"
#include "hx711_registers.h"
#include "cmsis_os2.h"

/* =========================================================================== */
/*                          ESTADO INTERNO POR CANAL                          */
/* =========================================================================== */

typedef struct {
    int32_t offset; /**< Valor crudo en vacío (tara).      */
    float   scale;  /**< Crudo / gramo (calibración).      */
} HX711_State;

static HX711_State s_state[HX711_CH_COUNT] = {
    { .offset = 0, .scale = HX711_SCALE_TANK_DEFAULT  },  /* TANK  */
    { .offset = 0, .scale = HX711_SCALE_PLATE_DEFAULT },  /* PLATE */
};

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
    HX711_port_init();

    /* Leer una vez cada canal para despertar el HX711 */
    for (uint8_t ch = 0; ch < HX711_CH_COUNT; ch++) {
        if (!HX711_port_wait_ready(ch)) return HX711_ERROR;
        (void)HX711_port_read_raw(ch);
    }

    return HX711_OK;
}

HX711_Status HX711_ReadGrams(uint8_t ch, float *grams)
{
    if (ch >= HX711_CH_COUNT || grams == NULL) return HX711_ERROR;

    int32_t avg;
    HX711_Status st = read_average(ch, &avg);
    if (st != HX711_OK) return st;

    float result = (float)(avg - s_state[ch].offset) / s_state[ch].scale;

    /* Clamp solo valores muy negativos (ruido de fondo) — tolerancia 5g */
    *grams = (result < -5.0f) ? 0.0f : result;

    return HX711_OK;
}

HX711_Status HX711_Tare(uint8_t ch)
{
    if (ch >= HX711_CH_COUNT) return HX711_ERROR;

    int32_t avg;
    HX711_Status st = read_average(ch, &avg);
    if (st != HX711_OK) return st;

    s_state[ch].offset = avg;
    return HX711_OK;
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