/**
 * @file    oled_registers.h
 * @brief   Comandos y constantes para display OLED SSD1306/SSD1315 128x64 vía I²C.
 */

#ifndef OLED_REGISTERS_H
#define OLED_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Dirección I²C del SSD1315                                                  */
/* D/C# = GND → 0x3C  |  D/C# = VCC → 0x3D                                  */
/* -------------------------------------------------------------------------- */
#define OLED_I2C_ADDR               0x3C

/* -------------------------------------------------------------------------- */
/* Bytes de control I²C                                                       */
/* -------------------------------------------------------------------------- */
#define OLED_CTRL_CMD               0x00  /**< Siguiente byte es comando.  */
#define OLED_CTRL_DATA              0x40  /**< Siguiente byte es dato.     */

/* -------------------------------------------------------------------------- */
/* Comandos fundamentales                                                      */
/* -------------------------------------------------------------------------- */
#define OLED_CMD_DISPLAY_OFF        0xAE
#define OLED_CMD_DISPLAY_ON         0xAF
#define OLED_CMD_DISPLAY_NORMAL     0xA6  /**< No inverso.                 */
#define OLED_CMD_DISPLAY_INVERSE    0xA7
#define OLED_CMD_ENTIRE_ON          0xA5  /**< Todos los px ON (test).     */
#define OLED_CMD_RESUME_RAM         0xA4  /**< Display desde RAM (normal). */
#define OLED_CMD_NOP                0xE3

/* -------------------------------------------------------------------------- */
/* Contraste                                                                   */
/* -------------------------------------------------------------------------- */
#define OLED_CMD_SET_CONTRAST       0x81
#define OLED_CONTRAST_DEFAULT       0xCF

/* -------------------------------------------------------------------------- */
/* Scroll                                                                      */
/* -------------------------------------------------------------------------- */
#define OLED_CMD_DEACTIVATE_SCROLL  0x2E

/* -------------------------------------------------------------------------- */
/* Modo de direccionamiento de memoria                                         */
/* -------------------------------------------------------------------------- */
#define OLED_CMD_SET_MEM_MODE       0x20
#define OLED_MEM_MODE_HORIZONTAL    0x00  /**< Auto-incrementa col→page.   */
#define OLED_MEM_MODE_VERTICAL      0x01
#define OLED_MEM_MODE_PAGE          0x02

#define OLED_CMD_SET_COL_ADDR       0x21  /**< Col start / end.            */
#define OLED_CMD_SET_PAGE_ADDR      0x22  /**< Page start / end.           */

/* -------------------------------------------------------------------------- */
/* Hardware / timing                                                           */
/* -------------------------------------------------------------------------- */
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3
#define OLED_CMD_SET_START_LINE     0x40  /**< OR con línea 0-63.          */
#define OLED_CMD_SET_MUX_RATIO      0xA8
#define OLED_MUX_64                 0x3F  /**< 64 líneas (display 128x64). */

#define OLED_CMD_SEG_REMAP_NORMAL   0xA0
#define OLED_CMD_SEG_REMAP_FLIP     0xA1  /**< Espejo horizontal.          */
#define OLED_CMD_COM_SCAN_NORMAL    0xC0
#define OLED_CMD_COM_SCAN_FLIP      0xC8  /**< Espejo vertical.            */

#define OLED_CMD_SET_COM_PINS       0xDA
#define OLED_COM_PINS_ALT           0x12  /**< Alternativo, sin remap.     */

#define OLED_CMD_SET_CLK_DIV        0xD5
#define OLED_CLK_DIV_DEFAULT        0x80

#define OLED_CMD_SET_PRECHARGE      0xD9
#define OLED_PRECHARGE_DEFAULT      0xF1  /**< Con charge pump interno.    */

#define OLED_CMD_SET_VCOMH          0xDB
#define OLED_VCOMH_DEFAULT          0x40

/* -------------------------------------------------------------------------- */
/* Charge pump interno (no necesita VCC externo)                              */
/* -------------------------------------------------------------------------- */
#define OLED_CMD_CHARGE_PUMP        0x8D
#define OLED_CHARGE_PUMP_ON         0x14
#define OLED_CHARGE_PUMP_OFF        0x10

/* -------------------------------------------------------------------------- */
/* Dimensiones                                                                 */
/* -------------------------------------------------------------------------- */
#define OLED_WIDTH                  128U
#define OLED_HEIGHT                 64U
#define OLED_PAGES                  8U    /**< 64px / 8px por página.      */
#define OLED_BUF_SIZE               (OLED_WIDTH * OLED_PAGES)  /**< 1024 B */

/* -------------------------------------------------------------------------- */
/* Fuente 6x8                                                                 */
/* -------------------------------------------------------------------------- */
#define OLED_FONT_W                 6U    /**< Ancho de carácter en px.    */
#define OLED_FONT_H                 8U    /**< Alto  de carácter en px.    */
#define OLED_CHARS_PER_LINE         (OLED_WIDTH / OLED_FONT_W)   /**< 21. */

/* -------------------------------------------------------------------------- */
/* Tiempos                                                                     */
/* -------------------------------------------------------------------------- */
#define OLED_DELAY_INIT_MS          100U  /**< Espera estabilización VCC.  */
#define OLED_SCREEN_SWITCH_MS       3000U /**< Alternancia pesos ↔ hora.   */

#ifdef __cplusplus
}
#endif

#endif /* OLED_REGISTERS_H */
