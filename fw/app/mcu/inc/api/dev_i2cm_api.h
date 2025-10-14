
#ifndef DEV_I2CM_API_H
#define DEV_I2CM_API_H

#include "api.h"

#define DEV_I2CM_API_ID 1

/* API Identifiers */
enum DEV_I2CM_API {
    I2CM_INITSTART = 0,
    I2CM_DEINIT = 1,
    I2CM_WRITE = 2,
    I2CM_READ = 3,
    I2CM_READ_SR = 4,
    I2CM_ISDEVICEREADY = 5,
};

/* API Function List */
#define DEV_I2CM_FUNCTIONS_COUNT 6
extern const FPTRS DEV_I2CM_FPTRS[DEV_I2CM_FUNCTIONS_COUNT];

/** Timeout de I2C en milisegundos. */
#define I2C_TIMEOUT (5000)

 

/**
*   Inicializa I2C1 a 400 kHz, 7-bit, sin dual address.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CM_InitStart (void);

 

/**
*   Desinicializa I2C1.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CM_DeInit (void);

 

/**
*   Escribe un buffer en un esclavo I²C.
*
* \return HAL_StatusTypeDef
*   \param [in] address (uint8_t) Dirección 7-bit del esclavo (p.ej. 0x68).
*   \param [in] data (uint8_t[512]) Puntero al buffer a transmitir.
*   \param [in] size (uint16_t) Cantidad de bytes a transmitir.
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CM_Write (uint8_t address, uint8_t* data, uint16_t size);

 

/**
*   Lee un buffer crudo desde un esclavo I²C (no lee dirección interna).
*
* \return HAL_StatusTypeDef
*   \param [in] address (uint8_t) Dirección 7-bit del esclavo (p.ej. 0x68).
*   \param [out] data (uint8_t[512]) Puntero al buffer de recepción.
*   \param [in] size (uint16_t) Cantidad de bytes a recibir.
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CM_Read (uint8_t address, uint8_t* data, uint16_t size);

 

/**
*   Lee bytes desde un registro interno (sub-address read).
*
* \return HAL_StatusTypeDef
*   \param [in] address (uint8_t) Dirección 7-bit del esclavo (p.ej. 0x68).
*   \param [in] reg (uint8_t) Dirección interna (8-bit) de inicio.
*   \param [out] data (uint8_t[512]) Buffer de salida.
*   \param [in] size (uint16_t) Número de bytes a leer.
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CM_Read_Sr (uint8_t address, uint8_t reg, uint8_t* data, uint16_t size);

 

/**
*   Verifica si un esclavo responde (ACK) en la dirección dada.
*
* \return HAL_StatusTypeDef
*   \param [in] address (uint8_t) Dirección 7-bit.
*   \param [in] trials (uint32_t) Número de reintentos.
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CM_IsDeviceReady (uint8_t address, uint32_t trials);

 
 

#endif // _DEV__API_H