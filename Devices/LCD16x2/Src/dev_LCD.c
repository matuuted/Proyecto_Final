/**
 * @file    dev_LCD.c
 * @brief   Librería de control para LCD 16x2 vía I²C (PCF8574) con RTOS.
 */

#include "dev_LCD.h"
#include <string.h>
#include <stdbool.h>

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

/* ----------------------- Variable privada ----------------------------------- */
// Control de backlight
static bool backlight_on = true;

/* -------------------------- Delay --------------------------------- */
static void LCD_delay(uint32_t ms) { osDelay(ms); }

/*  ---------------------- Funciones de Escritura ---------------------------- */
/**
 * @brief Mapeo de pines del PCF8574 hacia el LCD 16x2
 *
 * PCF8574   | LCD | Descripción
 * --------- | --- | ---------------------------------
 * P0        | RS  | 0 = comando, 1 = dato
 * P1        | RW  | 0 = escritura (fijo)
 * P2        | EN  | Habilita el latch del nibble (flanco ↑)
 * P3        | BL  | Backlight on/off
 * P4–P7     | D4–D7 | Bus de datos de 4 bits
 */

static HAL_StatusTypeDef LCD_send_pulse(uint8_t data)
{
    HAL_StatusTypeDef ret;
    uint8_t high = (data | LCD_EN);

    ret = LCD_register_write(high); /* Envio el nibble con EN=1 */
    if (ret != HAL_OK) return ret;

    LCD_delay(LCD_DELAY_CMD_MS);    /* Mantengo el pulso */

    uint8_t low = (data & ~LCD_EN); /* Envio el nibble con EN=0 */
    ret = LCD_register_write(low);
    if (ret != HAL_OK) return ret;

    LCD_delay(LCD_DELAY_CMD_MS);    /* Espero a que el LCD procese  */

    return HAL_OK;
}

/* ---------------- Escritura 4-bit ---------------- */
static HAL_StatusTypeDef LCD_write_4_bits(uint8_t nibble, uint8_t type)
{
    /* Envía un nibble (4 bits) por D4..D7, con RS según tipo (cmd/dato) y backlight si está habilitado. */
    
    uint8_t data = ((nibble & 0xF) << 4);       /* nibble a D4..D7 */
    if (type == LCD_REG_DATA) data |= LCD_RS;   /* RS: 0 = comando, 1 = dato */
    if (backlight_on) data |= LCD_BL;           /* Backlight */

    return LCD_send_pulse(data);
}

static HAL_StatusTypeDef LCD_send(uint8_t data, bool type)
{
    HAL_StatusTypeDef ret;

    /* Escribo el nibble alto */
    ret = LCD_write_4_bits((data >> 4), type);
    if (ret != HAL_OK) return ret;

    /* Escribo el nibble bajo */
    ret = LCD_write_4_bits((data & 0xF), type);
    return ret;
}


/* ------------------------- Funciones públicas ------------------------------ */
bool LCD_Init(void) {
    /* Secuencia estándar de wake-up del LCD, según el datasheet: 3 pulsos 0x3 + 0x2. */
    LCD_delay(LCD_DELAY_INIT1_MS);
    LCD_write_4_bits(LCD_WAKEUP1, LCD_REG_CMD);
    LCD_delay(LCD_DELAY_INIT2_MS);
    LCD_write_4_bits(LCD_WAKEUP1, LCD_REG_CMD);
    LCD_delay(LCD_DELAY_INIT2_MS);
    LCD_write_4_bits(LCD_WAKEUP1, LCD_REG_CMD);
    LCD_write_4_bits(LCD_WAKEUP2, LCD_REG_CMD);

    // Configura el formato del display:
    /* 4-bit, 2 líneas, 5x8; display ON; clear; entry mode. */
    LCD_send(LCD_FUNCTION_SET | 0x08, LCD_REG_CMD);
    LCD_send(LCD_DISPLAY_CTRL | LCD_DISPLAY_ON, LCD_REG_CMD);
    LCD_Clear();
    LCD_send(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT, LCD_REG_CMD); 

    return true;
}

void LCD_Clear(void) {
    /* El comando 0x01 borra DDRAM y manda el cursor a (0,0). */
    LCD_send(LCD_CLEAR_DISPLAY, LCD_REG_CMD);
    LCD_delay(LCD_DELAY_CLEAR_MS);
}

void LCD_SetCursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offsets[] = {LCD_ROW0_OFFSET, LCD_ROW1_OFFSET}; /* El array contiene el offset de las filas en DDRAM de mi LCD 16x2 */
    if (row > 1) row = 1;                               
    if (col >= DEV_LCD_COLS) col = DEV_LCD_COLS - 1 ;
    LCD_send(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_REG_CMD);
}

void LCD_WriteChar(char character) {
    /* Escribe un carácter en la posición actual */
    LCD_send((uint8_t)character, LCD_REG_DATA);
}

void LCD_Write(const char *s) {
    while (*s) {
        LCD_WriteChar(*s++);
    }
}

void LCD_WriteAt(uint8_t col, uint8_t row, char *s) {
    LCD_SetCursor(col, row);
    LCD_Write(s);
}

void LCD_PrintCentered(uint8_t row, char *s) {
    /* Calcula columna inicial para centrar 's' en 'width' y la imprime en 'row' */
    uint8_t len = strlen(s);
    uint8_t col = (DEV_LCD_COLS > len) ? ((DEV_LCD_COLS - len) / LCD_CENTER_DIVISOR) : LCD_START_COL_DEFAULT;
    LCD_SetCursor(col, row);
    LCD_Write(s);
}

void LCD_Cursor(bool enable) {
    if (enable) LCD_send(LCD_DISPLAY_CTRL | LCD_DISPLAY_ON | LCD_CURSOR_ON, LCD_REG_CMD);
    else        LCD_send(LCD_DISPLAY_CTRL | LCD_DISPLAY_ON, LCD_REG_CMD);
}

void LCD_BlinkCursor(bool enable) {
    if (enable) LCD_send(LCD_DISPLAY_CTRL | LCD_DISPLAY_ON | LCD_BLINK_ON, LCD_REG_CMD);
    else        LCD_send(LCD_DISPLAY_CTRL | LCD_DISPLAY_ON, LCD_REG_CMD);
}