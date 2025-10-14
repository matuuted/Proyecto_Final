/**
 * @file    mpu6050.c
 * @brief  Implementación del driver del MPU-6050.
 */

#include "mpu6050.h"          
#include "mpu6050_registers.h"
#include "mpu6050_port.h"

#define MPU6050_READ_LEN      14

/* -------------------------------------------------------------------------- */
/* Helpers internos                                                           */
/* -------------------------------------------------------------------------- */
static inline int16_t get_val(const uint8_t *data)
{
    return (int16_t)((data[0] << 8) | data[1]);
}

static float accel_sens_convert(uint8_t accel)
{
    switch (accel & MPU6050_ACCEL_FS_SEL_MASK)
    {
        case (0 << MPU6050_ACCEL_FS_SEL_SHIFT): return 1.0f/16384.0f; // ±2g
        case (1 << MPU6050_ACCEL_FS_SEL_SHIFT): return 1.0f/8192.0f;  // ±4g
        case (2 << MPU6050_ACCEL_FS_SEL_SHIFT): return 1.0f/4096.0f;  // ±8g
        default:                                return 1.0f/2048.0f;  // ±16g
    }
}

static float gyro_sens_convert(uint8_t gyro)
{
    switch (gyro & MPU6050_GYRO_FS_SEL_MASK)
    {
        case (0 << MPU6050_GYRO_FS_SEL_SHIFT): return 1.0f/131.0f; // ±250 dps
        case (1 << MPU6050_GYRO_FS_SEL_SHIFT): return 1.0f/65.5f;  // ±500 dps
        case (2 << MPU6050_GYRO_FS_SEL_SHIFT): return 1.0f/32.8f;  // ±1000 dps
        default:                               return 1.0f/16.4f;  // ±2000 dps
    }
}

static MPU6050_Status MPU6050_parse_hal_status(HAL_StatusTypeDef hal_status)
{
    switch (hal_status)
    {
        case HAL_OK:      return MPU6050_OK;
        case HAL_TIMEOUT: return MPU6050_TIMEOUT;
        case HAL_BUSY:    return MPU6050_BUSY;
        case HAL_ERROR:
        default:          return MPU6050_ERROR;
    }
}


static MPU6050_Status MPU6050_ReadRaw(MPU6050_DataRaw *raw)
{
    if (!raw) return MPU6050_INVALID_PARAM;

    uint8_t data[MPU6050_READ_LEN];
    MPU6050_Status ret = MPU6050_parse_hal_status(MPU6050_register_block_read(MPU6050_REG_ACCEL, data, sizeof(data)));
    if (ret != MPU6050_OK) return ret;

    raw->ax   = get_val(&data[MPU6050_AX]);
    raw->ay   = get_val(&data[MPU6050_AY]);
    raw->az   = get_val(&data[MPU6050_AZ]);
    raw->gx   = get_val(&data[MPU6050_GX]);
    raw->gy   = get_val(&data[MPU6050_GY]);
    raw->gz   = get_val(&data[MPU6050_GZ]);

    return MPU6050_OK;
}

/* -------------------------------------------------------------------------- */
/* API pública                                                                */
/* -------------------------------------------------------------------------- */

MPU6050_Status MPU6050_Init(MPU6050_Handler *device)
{
    if (!device) return MPU6050_INVALID_PARAM;
    
    uint8_t accel_cfg, gyro_cfg;

    /*  Despierto: CLKSEL = 1 (PLL Gyro X), SLEEP=0 */
    MPU6050_Status ret = MPU6050_parse_hal_status(MPU6050_register_write(MPU6050_REG_PWR_MGMT_1, ( 0x1 & MPU6050_PWR_CLKSEL_MASK )));
    if (ret != MPU6050_OK) return ret;

    /* Configuro el filtro digital pasa bajo ≈ 44Hz (valor típico = 3) */
    ret = MPU6050_parse_hal_status(MPU6050_register_write(MPU6050_REG_CONFIG, MPU6050_DLPF_44HZ));
    if (ret != MPU6050_OK) return ret;

    /* Leo accel_cfg / gyro_cfg para guardar las sensibilidades en la struct  */
    ret = MPU6050_parse_hal_status(MPU6050_register_read(MPU6050_REG_ACCEL_CONFIG, &accel_cfg));
    if (ret != MPU6050_OK) return ret;

    ret = MPU6050_parse_hal_status(MPU6050_register_read(MPU6050_REG_GYRO_CONFIG, &gyro_cfg));
    if (ret != MPU6050_OK) return ret;

    device->accel_sens = accel_sens_convert(accel_cfg);
    device->gyro_sens  = gyro_sens_convert(gyro_cfg);

    return MPU6050_OK;
}

MPU6050_Status MPU6050_Read(MPU6050_Handler *device)
{
    if (!device) return MPU6050_INVALID_PARAM;

    MPU6050_DataRaw raw;
    MPU6050_Status ret = MPU6050_ReadRaw(&raw);
    if (ret != MPU6050_OK) return ret;

    device->data.accel_x = raw.ax * device->accel_sens;
    device->data.accel_y = raw.ay * device->accel_sens;
    device->data.accel_z = raw.az * device->accel_sens;

    device->data.gyro_x = raw.gx * device->gyro_sens;
    device->data.gyro_y = raw.gy * device->gyro_sens;
    device->data.gyro_z = raw.gz * device->gyro_sens;

    return MPU6050_OK;
}