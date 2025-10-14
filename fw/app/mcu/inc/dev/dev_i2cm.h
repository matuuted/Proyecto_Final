/**
 * @file    dev_i2cm.h
 * @brief   Capa de I2C Master (HAL) para STM32.
 * @author  > Matias D. <
 * @date    2025
 *
 * @details
 *  Contiene operaciones I²C Master del proyecto.
 *
 * @note
 *  - La dirección del esclavo se pasa en 7-bit (p.ej. 0x68) y la capa hace (addr<<1).
 *  - Timeout por defecto: I2C_TIMEOUT (ms).
 */

#ifndef DEV_I2CM_H
#define DEV_I2CM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup DEV_I2CM I2C Master (HAL)
 *  @brief Funciones de inicialización del I²C
 *  @{
 */

#ifndef I2C_TIMEOUT
/** Timeout de I2C en milisegundos */
#define I2C_TIMEOUT           (5000)
#endif

/**
 * @brief  Inicializa I2C1 a 400 kHz, 7-bit, sin dual address.
 * @return HAL_OK si se configuró correctamente.
 */
HAL_StatusTypeDef I2CM_InitStart(void);

/**
 * @brief  Desinicializa I2C1.
 * @return HAL_OK si se desinicializó correctamente.
 */
HAL_StatusTypeDef I2CM_DeInit(void);

/**
 * @brief  Escribe un buffer en un esclavo I²C.
 * @param  address  Dirección 7-bit (p.ej. 0x68).
 * @param  data     Puntero a buffer a transmitir.
 * @param  size     Cantidad de bytes a transmitir.
 * @return HAL_OK si finalizó correctamente.
 */
HAL_StatusTypeDef I2CM_Write(uint8_t address, uint8_t *data, uint16_t size);

/**
 * @brief  Lee un buffer crudo desde un esclavo I²C (no lee dirección interna).
 * @param  address  Dirección 7-bit (p.ej. 0x68).
 * @param  data     Puntero a buffer de recepción.
 * @param  size     Cantidad de bytes a recibir.
 * @return HAL_OK si finalizó correctamente.
 */
HAL_StatusTypeDef I2CM_Read(uint8_t address, uint8_t *data, uint16_t size);

/**
 * @brief  Lee bytes desde un registro interno.
 * @param  address  Dirección 7-bit (p.ej. 0x68).
 * @param  reg      Dirección interna (8-bit) de inicio.
 * @param  data     Buffer de salida.
 * @param  size     Número de bytes a leer.
 * @return HAL_OK si finalizó correctamente.
 */
HAL_StatusTypeDef I2CM_Read_Sr(uint8_t address, uint8_t reg, uint8_t *data, uint16_t size);

/**
 * @brief  Verifica si un esclavo responde (ACK) en la dirección dada.
 * @param  address  Dirección 7-bit.
 * @param  trials   Numero de reintentos.
 * @return HAL_OK si respondió el esclavo a la direccion dada.
 */
HAL_StatusTypeDef I2CM_IsDeviceReady(uint8_t address, uint32_t trials);

/** @} */ // end group DEV_I2CM

#ifdef __cplusplus
}
#endif

#endif /* DEV_I2CM_H */
