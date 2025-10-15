/**
 * @file    mpu6050_registers.h
 * @brief   Definiciones de registros y mascaras para el MPU6050.
 * @details
 *  Este archivo contiene las direcciones de los registros y máscaras de bits
 *  usadas por el driver mpu6050.
 */

#ifndef MPU6050_REGS_H
#define MPU6050_REGS_H


/* ---------------- Registros utilizados ---------------- */
#define MPU6050_REG_CONFIG         0x1A
#define MPU6050_REG_GYRO_CONFIG    0x1B
#define MPU6050_REG_ACCEL_CONFIG   0x1C
#define MPU6050_REG_ACCEL          0x3B
#define MPU6050_REG_PWR_MGMT_1     0x6B

/* --------------- Máscaras / shifts utilizados ---------- */
/* PWR_MGMT_1 */
#define MPU6050_PWR_CLKSEL_MASK    0x07 /* 0: Int 8MHz, 1: PLL X-gyro, etc. */

/* GYRO_CONFIG */
#define MPU6050_GYRO_FS_SEL_SHIFT  3
#define MPU6050_GYRO_FS_SEL_MASK   (0x03 << MPU6050_GYRO_FS_SEL_SHIFT) /* 0:250,1:500,2:1000,3:2000 dps */

/* ACCEL_CONFIG */
#define MPU6050_ACCEL_FS_SEL_SHIFT 3
#define MPU6050_ACCEL_FS_SEL_MASK  (0x03 << MPU6050_ACCEL_FS_SEL_SHIFT) /* 0:±2g,1:±4g,2:±8g,3:±16g */

/* FILTRO DIGITAL    */
#define MPU6050_DLPF_44HZ        3

enum {
  MPU6050_AX = 0,
  MPU6050_AY = 2, 
  MPU6050_AZ = 4, 
  MPU6050_GX = 8, 
  MPU6050_GY = 10, 
  MPU6050_GZ = 12, 
};

#endif // MPU6050_REGS_H