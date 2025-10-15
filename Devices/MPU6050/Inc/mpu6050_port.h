/**
 * @file    MPU6050_port.h
 * @brief   Driver del MPU6050 utilizando dev_i2cm.
 * @details
 *  API utilizada para leer y escribir registros del MPU6050.
 */

#ifndef MPU6050_PORT_H
#define MPU6050_PORT_H

#include <stdint.h>
#include "dev_i2cm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MPU6050_PORT
 *  @{
 */

 /** Dirección I2C del MPU6050 (0x69) */
#define MPU6050_ADDRESS        (0x69)

/**< Cantidad de reintentos en I2CM_IsDeviceReady */
#define MPU6050_RETRY_COUNT      (3)     

/**
 * @brief  Verifica la presencia del MPU6050 en el bus I2C.
 * @return Retorna HAL_OK si responde al address 0x69.
 */
HAL_StatusTypeDef MPU6050_is_ready(void);

/**
 * @brief  Escribe un registro del MPU6050.
 * @param  reg   Dirección del registro.
 * @param  data  Dato a escribir.
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef MPU6050_register_write(uint8_t reg, uint8_t data);

/**
 * @brief  Escribe un bloque de datos del MPU6050.
 * @param  data  Puntero al bloque de datos a escribir.
 * @param  len   Longitud del bloque de datos.
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef MPU6050_register_block_write(uint8_t *data, uint16_t len);

/**
 * @brief  Lee un registro del MPU6050.
 * @param  reg   Dirección del registro.
 * @param  data  Puntero de salida (1 byte).
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef MPU6050_register_read(uint8_t reg, uint8_t *data);

/**
 * @brief  Lee un registro del MPU6050.
 * @param  reg   Dirección del registro donde se comienza a leer.
 * @param  data  Puntero de salida.
 * @param  len   Cantidad de registros a leer.
 * @return HAL_OK si funciono correctamente.
 */
HAL_StatusTypeDef MPU6050_register_block_read(uint8_t reg, uint8_t *data, uint16_t len);

/** @} */ // end group MPU6050_PORT

#ifdef __cplusplus
}
#endif

#endif /* MPU6050_PORT_H */