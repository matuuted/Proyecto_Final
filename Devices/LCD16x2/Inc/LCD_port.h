/**
 * @file    LCD_port.h
 * @brief   Primitivas de bajo nivel para LCD HD44780 vía PCF8574.
 */
#ifndef LCD_PORT_H
#define LCD_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "dev_i2cm.h"
#include "LCD_registers.h"

#ifndef LCD_RETRY_COUNT
    #define LCD_RETRY_COUNT   2
#endif

/* Escritura */
HAL_StatusTypeDef LCD_register_write(uint8_t value); 

#ifdef __cplusplus
}
#endif
#endif /* LCD_PORT_H */