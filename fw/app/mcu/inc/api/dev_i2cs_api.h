
#ifndef DEV_I2CS_API_H
#define DEV_I2CS_API_H

#include "api.h"

#define DEV_I2CS_API_ID 0

/* API Identifiers */
enum DEV_I2CS_API {
    I2CS_INITSTART = 0,
    I2CS_DEINIT = 1,
    I2C2_ENABLEACKNOWLEDGE = 2,
    I2C2_DISABLEACKNOWLEDGE = 3,
};

/* API Function List */
#define DEV_I2CS_FUNCTIONS_COUNT 4
extern const FPTRS DEV_I2CS_FPTRS[DEV_I2CS_FUNCTIONS_COUNT];

/** I2C slave address. */
#define I2CS_SLAVE_ADDR 0x50

 
/** I2C slave address. */
enum I2CS_STATE {
    /** I2C slave is ready. */
    I2CS_STATE_READY = 0,
    /** I2C slave is getting address. */
    I2CS_STATE_GET_ADDR = 1,
    /** I2C slave is setting address. */
    I2CS_STATE_SET_ADDR = 2,
    /** I2C slave is getting data. */
    I2CS_STATE_GET_DATA = 3,
    /** I2C slave is sending data. */
    I2CS_STATE_SET_DATA = 4,
    /** I2C slave is not acknowledging. */
    I2CS_STATE_NACK = 5,
};

 

/**
*   Initializes the I2C slave module.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CS_InitStart (void);

 

/**
*   Deinitializes the I2C slave module.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2CS_DeInit (void);

 

/**
*   Enables the I2C slave module.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2C2_EnableAcknowledge (void);

 

/**
*   Disables the I2C slave module.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: Any
**/
HAL_StatusTypeDef I2C2_DisableAcknowledge (void);

 
 

#endif // _DEV__API_H