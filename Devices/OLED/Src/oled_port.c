/**
 * @file    oled_port.c
 * @brief   Primitivas de bajo nivel para OLED SSD1315 vía I²C1.
 * @details
 *  Toda la comunicación física pasa por I2CM_Write() de dev_i2cm.c.
 *  El SSD1315 espera: [dirección I²C] [byte control] [dato(s)].
 */

#include "oled_port.h"

HAL_StatusTypeDef OLED_port_send_cmd(uint8_t cmd)
{
    uint8_t buf[2] = { OLED_CTRL_CMD, cmd };
    return I2CM_Write(OLED_I2C_ADDR, buf, 2);
}

HAL_StatusTypeDef OLED_port_send_cmd2(uint8_t cmd, uint8_t arg)
{
    uint8_t buf[3] = { OLED_CTRL_CMD, cmd, arg };
    return I2CM_Write(OLED_I2C_ADDR, buf, 3);
}

HAL_StatusTypeDef OLED_port_flush(uint8_t *buf, uint16_t len)
{
    return I2CM_Write(OLED_I2C_ADDR, buf, len);
}
