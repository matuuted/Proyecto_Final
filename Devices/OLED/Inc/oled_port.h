/**
 * @file    oled_port.h
 * @brief   Primitivas de bajo nivel para OLED SSD1315 vía I²C1 (PB6/PB7).
 * @details
 *  Usa I2CM_Write() de dev_i2cm.h — mismo bus que el DS3231.
 */

#ifndef OLED_PORT_H
#define OLED_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "dev_i2cm_api.h"
#include "oled_registers.h"

/**
 * @brief  Envía un comando al SSD1315.
 * @param  cmd  Byte de comando.
 * @return HAL_OK si el I²C respondió con ACK.
 */
HAL_StatusTypeDef OLED_port_send_cmd(uint8_t cmd);

/**
 * @brief  Envía un comando de dos bytes (cmd + argumento).
 */
HAL_StatusTypeDef OLED_port_send_cmd2(uint8_t cmd, uint8_t arg);

/**
 * @brief  Envía el buffer completo de datos de pantalla (1025 bytes).
 * @param  buf  Puntero al buffer: buf[0] = OLED_CTRL_DATA, buf[1..1024] = píxeles.
 * @param  len  Longitud total (1 + OLED_BUF_SIZE).
 * @return HAL_OK si OK.
 */
HAL_StatusTypeDef OLED_port_flush(uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* OLED_PORT_H */
