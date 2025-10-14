/**
 * @file    mpu6050_port.c
 * @brief   Implementación del driver MPU6050 usando dev_i2cm (HAL I²C).
 */

#include "mpu6050_port.h"

/* -------------------------------------------------------------------------- */
/*  API pública                                                               */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef MPU6050_is_ready(void)
{
    return I2CM_IsDeviceReady(MPU6050_ADDRESS, MPU6050_RETRY_COUNT);
}

HAL_StatusTypeDef MPU6050_register_write(uint8_t reg, uint8_t data)
{
    //Genero el buffer con la direccion del registro y el byte de data.
    uint8_t buf[2] = { reg, data };
    return I2CM_Write(MPU6050_ADDRESS, buf, 2);
}

HAL_StatusTypeDef MPU6050_register_block_write(uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return HAL_ERROR;

    return I2CM_Write(MPU6050_ADDRESS, data, len);
}

HAL_StatusTypeDef MPU6050_register_read(uint8_t reg, uint8_t *data)
{
    if (!data) return HAL_ERROR;
    return I2CM_Read_Sr(MPU6050_ADDRESS, reg, data, 1);
}

HAL_StatusTypeDef MPU6050_register_block_read(uint8_t reg, uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return HAL_ERROR;
    return I2CM_Read_Sr(MPU6050_ADDRESS, reg, data, len);
}