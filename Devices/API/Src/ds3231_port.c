/**
 * @file    ds3231_port.c
 * @brief   Implementación del driver DS3231 usando dev_i2cm (HAL I²C).
 */

#include "ds3231_port.h"

/* -------------------------------------------------------------------------- */
/*  API pública                                                               */
/* -------------------------------------------------------------------------- */

HAL_StatusTypeDef DS3231_is_ready(void)
{
    return I2CM_IsDeviceReady(DS3231_ADDRESS, DS3231_RETRY_COUNT);
}

HAL_StatusTypeDef DS3231_register_write(uint8_t reg, uint8_t data)
{
    //Genero el buffer con la direccion del registro y el byte de data.
    uint8_t buf[2] = { reg, data };
    return I2CM_Write(DS3231_ADDRESS, buf, 2);
}

HAL_StatusTypeDef DS3231_register_block_write(uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return HAL_ERROR;

    return I2CM_Write(DS3231_ADDRESS, data, len);
}

HAL_StatusTypeDef DS3231_register_read(uint8_t reg, uint8_t *data)
{
    if (!data) return HAL_ERROR;
    return I2CM_Read_Sr(DS3231_ADDRESS, reg, data, 1);
}

HAL_StatusTypeDef DS3231_register_block_read(uint8_t reg, uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return HAL_ERROR;
    return I2CM_Read_Sr(DS3231_ADDRESS, reg, data, len);
}