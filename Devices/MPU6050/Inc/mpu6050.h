/**
 * @file    MPU6050.h
 * @brief   Driver del MPU6050 utilizando dev_i2cm.
 * @details
 *  API utilizada para leer y escribir registros del MPU6050.
 */

#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "mpu6050_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* ENUMERACIONES Y ESTRUCTURAS                                                          */
/* -------------------------------------------------------------------------- */
typedef enum {
    MPU6050_OK              =  0, // OK
    MPU6050_ERROR           = -1, // Error genérico de HAL
    MPU6050_TIMEOUT         = -2, // Timeout de comunicación I2C
    MPU6050_NOT_READY       = -3, // El MPU6050 no respondió al address
    MPU6050_INVALID_PARAM   = -4, // Parámetros inválidos
    MPU6050_BUSY            = -5, // HAL Busy
} MPU6050_Status;

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;    
} MPU6050_Data;

typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
} MPU6050_DataRaw;

typedef struct {
    float accel_sens;               /**< Sensibilidad del acelerómetro [g/LSB] */
    float gyro_sens;                /**< Sensibilidad del giroscopio [°/s/LSB] */
    MPU6050_Data data;              /**< Última lectura de informacion         */
} MPU6050_Handler;

/* -------------------------------------------------------------------------- */
/* API PÚBLICA                                                                */
/* -------------------------------------------------------------------------- */

MPU6050_Status MPU6050_Init(MPU6050_Handler *device);
MPU6050_Status MPU6050_Read(MPU6050_Handler*device);

#ifdef __cplusplus
}
#endif

#endif /* MPU6050_H */