/**
 * @file    LCD_port.c
 * @brief   Implementación de primitivas de bajo nivel para LCD HD44780 vía PCF8574.
 */
#include "LCD_port.h"

HAL_StatusTypeDef LCD_is_ready(void)
{
    return I2CM_IsDeviceReady(LCD_I2C_ADDR, LCD_RETRY_COUNT);
}

HAL_StatusTypeDef LCD_register_write(uint8_t value)
{
    return I2CM_Write(LCD_I2C_ADDR, &value, 1);
}
