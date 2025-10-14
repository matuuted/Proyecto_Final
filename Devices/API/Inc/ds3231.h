/**
 * @file    ds3231.h
 * @brief   Driver del RTC DS3231 utilizando dev_i2cm.
 * @details
 *  API utilizada para leer y escribir registros del RTC DS3231.
 */

#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* ENUMS Y STRUCTS                                                             */
/* -------------------------------------------------------------------------- */

typedef enum {
    DS3231_OK               =  0, // OK
    DS3231_ERROR            = -1, // Error genérico de HAL
    DS3231_TIMEOUT          = -2, // Timeout de comunicación I2C
    DS3231_NOT_READY        = -3, // El DS3231 no respondió al address
    DS3231_INVALID_PARAM    = -4, // Parámetros inválidos
    DS3231_BUSY             = -5, // HAL Busy
} DS3231_Status;

/**
 * @brief Estructura de tiempo.
 */
typedef struct {
    uint8_t seconds; /**< 0x00: segundos */
    uint8_t minutes; /**< 0x01: minutos  */
    uint8_t hours;   /**< 0x02: horas    */
    uint8_t day;     /**< 0x03: día de semana */
    uint8_t date;    /**< 0x04: día del mes */
    uint8_t month;   /**< 0x05: mes, bit7=Century */
    uint8_t year;    /**< 0x06: año (00..99) */
} DS3231_Time;

/** @name Convertidores BCD
 *  @brief Estas funciones convierten números entre decimal normal y BCD (Binary Coded Decimal), que es el formato que usa el DS3231 para guardar hora y fecha.
 * 
 * dec2bcd:
 *  - Convierte un número decimal (ej: 45) a BCD (0x45).
 *  Las decenas van al nibble alto (bits 7-4).
 *  Las unidades van al nibble bajo (bits 3-0).
 * 
 * bcd2dec:
 * - Convierte de BCD (0x45) a decimal (45).
 *  Toma el nibble alto, lo multiplica por 10 y le suma el nibble bajo.
 */
static inline uint8_t dec2bcd(uint8_t value) { 
    return (uint8_t)(((value/10) << 4) | (value % 10)); 
}
static inline uint8_t bcd2dec(uint8_t value) { 
    return (uint8_t)(((value >> 4) * 10) + (value & 0x0F)); 
}

/* -------------------------------------------------------------------------- */
/* API pública                                                                */
/* -------------------------------------------------------------------------- */

/**
 * @brief Verifica la presencia del DS3231 en el bus de I2C.
 * @param  void.
 * @return DS3231_OK si funciono correctamente.
 */
DS3231_Status DS3231_Init(void);

/**
 * @brief  Lee la hora actual desde el DS3231.
 * @param  time  Estructura Time.
 * @return DS3231_OK si funciono correctamente.
 */
DS3231_Status DS3231_ReadTime(DS3231_Time *time);


/**
 * @brief  Configura la fecha y hora del RTC.
 * @param  yy   Año (00-99)
 * @param  mm   Mes (1-12)
 * @param  dd   Día del mes (1-31)
 * @param  dow  Día de semana (1-7)
 * @param  hh   Hora (0-23)
 * @param  mi   Minutos (0-59)
 * @param  ss   Segundos (0-59)
 * @return DS3231_OK si funciono correctamente.
 */
DS3231_Status DS3231_SetTime(uint8_t year, uint8_t month, uint8_t date, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

/** @example
 *  @code
 *  if (DS3231_Init() == DS3231_OK) {
 *      (void)DS3231_SetTime(25, 9, 24, 3, 16, 20, 30); // 2025-09-24 16:20:30
 *      DS3231_Time now;
 *      if (DS3231_ReadTime(&now) == DS3231_OK) {
 *          uint8_t hh = now.hours;
 *          uint8_t mi = now.minutes;
 *          uint8_t ss = now.seconds;
 *      }
 *  }
 *  @endcode
 */

#ifdef __cplusplus
}
#endif

#endif /* DS3231_H */