/**
 * @file    ds3231_port.h
 * @brief   Driver del RTC DS3231 utilizando dev_i2cm.
 * @details
 *  API utilizada para leer y escribir registros del RTC DS3231.
 */

#ifndef DS3231_PORT_H
#define DS3231_PORT_H

#include <stdint.h>
#include "dev_i2cm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup RTC DS3231_PORT
 *  @{
 */

 /** Dirección 7-bit del DS3231 (0x68) */
#define DS3231_ADDRESS        (0x68)

/**< Cantidad de reintentos en I2CM_IsDeviceReady */
#define DS3231_RETRY_COUNT      (3)     

/**
 * @brief  Verifica la presencia del RTC DS3231 en el bus I2C.
 * @return Retorna HAL_OK si responde al address 0x68.
 */
HAL_StatusTypeDef DS3231_is_ready(void);

/**
 * @brief  Escribe un registro del DS3231.
 * @param  reg   Dirección del registro.
 * @param  data  Dato a escribir.
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef DS3231_register_write(uint8_t reg, uint8_t data);

/**
 * @brief  Escribe un bloque de datos del DS3231.
 * @param  data  Puntero al bloque de datos a escribir.
 * @param  len   Longitud del bloque de datos.
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef DS3231_register_block_write(uint8_t *data, uint16_t len);

/**
 * @brief  Lee un registro del DS3231.
 * @param  reg   Dirección del registro.
 * @param  data  Puntero de salida (1 byte).
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef DS3231_register_read(uint8_t reg, uint8_t *data);

/**
 * @brief  Lee un registro del DS3231.
 * @param  reg   Dirección del registro donde se comienza a leer.
 * @param  data  Puntero de salida.
 * @param  len   Cantidad de registros a leer.
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef DS3231_register_block_read(uint8_t reg, uint8_t *data, uint16_t len);

/** @} */ // end group DS3231_PORT

#ifdef __cplusplus
}
#endif

#endif /* DS3231_PORT_H */