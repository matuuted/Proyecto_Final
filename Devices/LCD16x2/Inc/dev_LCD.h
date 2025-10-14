/**
 * @file    dev_LCD.h
 * @brief   Driver para LCD 16x2 (HD44780) vía I²C con backpack PCF8574.
 * @details
 *  API para inicializar el display LCD, posicionar el cursor y escribir texto. 
 */

#ifndef DEV_LCD_H
#define DEV_LCD_H

#include <stdint.h>
#include <stdbool.h>
#include "LCD_port.h"    

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* CONFIGURACIÓN DEL MÓDULO                                                   */
/* -------------------------------------------------------------------------- */

/**
 * @brief Cantidad de columnas del display.
 */
#ifndef DEV_LCD_COLS
#define DEV_LCD_COLS 16
#endif

/**
 * @brief Cantidad de filas del display.
 */
#ifndef DEV_LCD_ROWS
#define DEV_LCD_ROWS 2
#endif

/**
 * @brief Columna inicial por del display.
 */
#define DEV_LCD_COLS_0 0

/**
 * @brief Primera fila del display.
 */
#define DEV_LCD_ROWS_0 0

/**
 * @brief Segunda fila del display.
 */
#define DEV_LCD_ROWS_1 1


/* -------------------------------------------------------------------------- */
/* API PÚBLICA                                                                */
/* -------------------------------------------------------------------------- */

/**
 * @brief Inicializa el LCD.
 * @details
 *  1. Ejecuta la secuencia de wake-up, 
 *  2. Envía Function Set,
 *  3. Enciende el display 
 *  4. Limpia y configura el entry mode.
 * @return true si completó la secuencia correctamente.
 */
bool LCD_Init(void);

/**
 * @brief Limpia el display y posiciona el cursor en (0,0).
 * @details
 *  Envía `0x01` (Clear Display). 
 * @note Requiere delay de (≈2 ms).
 */
void LCD_Clear(void);

/**
 * @brief Posiciona el cursor en (col, row).
 * @param col Columna (0..DEV_LCD_COLS-1)
 * @param row Fila    (0..DEV_LCD_ROWS-1)
 * @note  Usa las direcciones base de DDRAM por fila (ej. 0x00 y 0x40 en 16x2).
 */
void LCD_SetCursor(uint8_t col, uint8_t row);

/**
 * @brief Escribe un carácter en la posición actual del cursor.
 * @param c Carácter ASCII a imprimir.
 */
void LCD_WriteChar(char c);

/**
 * @brief Escribe una cadena ASCII (terminada en '\0').
 * @param s Puntero a cadena.
 */
void LCD_Write(const char *s);

/**
 * @brief Escribe una cadena en la posición determinada (col,row).
 * @param col Columna a donde se va a escribir la cadena.
 * @param row Fila a donde se va a escribir la cadena.
 * @param s   Puntero a cadena.
 */
void LCD_WriteAt(uint8_t col, uint8_t row, char *s);

/**
 * @brief Centra un texto en la fila indicada.
 * @param row   Fila destino.
 * @param s     Cadena a centrar.
 */
void LCD_PrintCentered(uint8_t row, char *s);

/**
 * @brief Habilita u deshabilita el cursor.
 * @param enable true: cursor visible; false: oculto.
 */
void LCD_Cursor(bool enable);

/**
 * @brief Habilita o deshabilita el parpadeo del cursor.
 * @param enable true: parpadeo ON; false: OFF.
 */
void LCD_BlinkCursor(bool enable);


#ifdef __cplusplus
}
#endif

#endif /* DEV_LCD_H */