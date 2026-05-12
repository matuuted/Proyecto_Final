/**
 * @file    hx711_port.c
 * @brief   Primitivas de bajo nivel para HX711 vía GPIO bit-bang.
 */

#include "hx711_port.h"
#include "cmsis_os2.h"
/* =========================================================================== */
/*                          TABLA DE PINES POR CANAL                          */
/* =========================================================================== */

typedef struct {
    GPIO_TypeDef *dout_port;
    uint16_t      dout_pin;
    GPIO_TypeDef *sck_port;
    uint16_t      sck_pin;
} HX711_PinMap;

static const HX711_PinMap s_pins[HX711_CH_COUNT] = {
    /* TANK */
    { HX711_TANK_DOUT_PORT,  HX711_TANK_DOUT_PIN,
      HX711_TANK_SCK_PORT,   HX711_TANK_SCK_PIN  },
    /* PLATE */
    { HX711_PLATE_DOUT_PORT, HX711_PLATE_DOUT_PIN,
      HX711_PLATE_SCK_PORT,  HX711_PLATE_SCK_PIN },
};

/* =========================================================================== */
/*                          DELAY ~1µs                                         */
/* =========================================================================== */

static inline void delay_us(uint32_t us)
{
    volatile uint32_t count = us * 21U; // ~84 MHz / 4 ciclos por iteración
    while (count--) { __NOP(); }
}

/* =========================================================================== */
/*                          API                                                 */
/* =========================================================================== */

void HX711_port_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* SCK como salidas push-pull */
    gpio.Pin   = HX711_TANK_SCK_PIN | HX711_PLATE_SCK_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio);

    /* DOUT como entradas sin pull */
    gpio.Pin  = HX711_TANK_DOUT_PIN | HX711_PLATE_DOUT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &gpio);

    /* SCK en LOW → chips activos */
    HAL_GPIO_WritePin(HX711_TANK_SCK_PORT,  HX711_TANK_SCK_PIN,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(HX711_PLATE_SCK_PORT, HX711_PLATE_SCK_PIN, GPIO_PIN_RESET);
}

bool HX711_port_wait_ready(uint8_t ch)
{
    if (ch >= HX711_CH_COUNT) return false;
    const HX711_PinMap *p = &s_pins[ch];
    uint32_t start = HAL_GetTick();

    while (HAL_GPIO_ReadPin(p->dout_port, p->dout_pin) == GPIO_PIN_SET) {
        if ((HAL_GetTick() - start) > HX711_READY_TIMEOUT_MS)
            return false;
        osDelay(1);
    }
    return true;
}

uint32_t HX711_port_read_raw(uint8_t ch)
{
    if (ch >= HX711_CH_COUNT) return 0;
    const HX711_PinMap *p = &s_pins[ch];
    uint32_t raw = 0;

    /* 24 pulsos de datos */
    for (int i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(p->sck_port, p->sck_pin, GPIO_PIN_SET);
        delay_us(1);
        raw <<= 1;
        if (HAL_GPIO_ReadPin(p->dout_port, p->dout_pin) == GPIO_PIN_SET)
            raw |= 1U;
        HAL_GPIO_WritePin(p->sck_port, p->sck_pin, GPIO_PIN_RESET);
        delay_us(1);
    }

    /* Pulso 25 → selecciona Canal A Gain 128 para la próxima lectura */
    HAL_GPIO_WritePin(p->sck_port, p->sck_pin, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(p->sck_port, p->sck_pin, GPIO_PIN_RESET);
    delay_us(1);

    return raw;
}

void HX711_port_power_down(uint8_t ch)
{
    if (ch >= HX711_CH_COUNT) return;
    /* SCK en HIGH por >60µs → power-down */
    HAL_GPIO_WritePin(s_pins[ch].sck_port, s_pins[ch].sck_pin, GPIO_PIN_SET);
    osDelay(1);
}

void HX711_port_power_up(uint8_t ch)
{
    if (ch >= HX711_CH_COUNT) return;
    HAL_GPIO_WritePin(s_pins[ch].sck_port, s_pins[ch].sck_pin, GPIO_PIN_RESET);
}
