/**
 * @file    dev_LCD_registers.h
 * @brief   Definiciones de comandos y flags para LCD 16x2 vía I²C.
 */

#ifndef DEV_LCD_REGISTERS_H
#define DEV_LCD_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Dirección del módulo I²C (PCF8574)                                         */
/* -------------------------------------------------------------------------- */
#define LCD_I2C_ADDR        (0x27)

/* -------------------------------------------------------------------------- */
/* Comandos básicos                                                           */
/* -------------------------------------------------------------------------- */
#define LCD_CLEAR_DISPLAY     0x01
#define LCD_RETURN_HOME       0x02
#define LCD_ENTRY_MODE_SET    0x04
#define LCD_DISPLAY_CTRL      0x08
#define LCD_FUNCTION_SET      0x20
#define LCD_SET_DDRAM_ADDR    0x80

/* -------------------------------------------------------------------------- */
/* Flags de uso común                                                         */
/* -------------------------------------------------------------------------- */
#define LCD_ENTRY_LEFT        0x02
#define LCD_DISPLAY_ON        0x04
#define LCD_CURSOR_ON         0x02
#define LCD_BLINK_ON          0x01

/* -------------------------------------------------------------------------- */
/* Bits de control del PCF8574                                               */
/* -------------------------------------------------------------------------- */
#define LCD_RS              0x01   /* Register Select */
#define LCD_EN              0x04   /* Enable */
#define LCD_BL              0x08   /* Backlight */

/* -------------------------------------------------------------------------- */
/* Tiempos mínimos de espera (en ms)                                         */
/* -------------------------------------------------------------------------- */
#define LCD_DELAY_INIT1_MS  50
#define LCD_DELAY_INIT2_MS  5
#define LCD_DELAY_CLEAR_MS  2
#define LCD_DELAY_CMD_MS    1

/* -------------------------------------------------------------------------- */
/* Secuencia de wake-up                                                     */
/* -------------------------------------------------------------------------- */
#define LCD_WAKEUP1         0x03
#define LCD_WAKEUP2         0x02

/* -------------------------------------------------------------------------- */
/* Direcciones base de DDRAM (inicio de fila)                                 */
/* -------------------------------------------------------------------------- */
#define LCD_ROW0_OFFSET   0x00
#define LCD_ROW1_OFFSET   0x40

/* -------------------------------------------------------------------------- */
/* Valores utilizados para centrar el string.                                 */
/* -------------------------------------------------------------------------- */
#define LCD_CENTER_DIVISOR       2   /* Divide el espacio libre en dos para centrar */
#define LCD_START_COL_DEFAULT    0   /* Columna inicial, utilizada cuando el texto no entra */

/* -------------------------------------------------------------------------- */
/* Tipo de data a enviar (comando/dato)                                       */
/* -------------------------------------------------------------------------- */
#define LCD_REG_CMD             0x00
#define LCD_REG_DATA            0x01

#ifdef __cplusplus
}
#endif

#endif /* DEV_LCD_REGISTERS_H */
