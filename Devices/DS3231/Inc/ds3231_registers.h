/**
 * @file    ds3231_registers.h
 * @brief   Definiciones de registros y mascaras para el RTC DS3231.
 * @details
 *  Este archivo contiene las direcciones de los registros y máscaras de bits
 *  usadas por el driver DS3231.
 */

#ifndef DS3231_REGISTERS_H
#define DS3231_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup DS3231_registers.h Definiciones de registros
 *  @{
 */

/* Tamaños definidos para buffers. */
#define DS3231_MAX_BLOCK_WRITE   (8)
#define DS3231_MAX_BLOCK_READ    (7)

/* -------------------------------------------------------------------------- */
/* Direcciones de Registros del DS3231                                        */
/* -------------------------------------------------------------------------- */
/**
 * Nota: Los registros de hora y fecha están en formato BCD
 */
#define DS3231_REG_SECONDS       (0x00)
#define DS3231_REG_MINUTES       (0x01)
#define DS3231_REG_HOURS         (0x02)
#define DS3231_REG_DAY           (0x03)  /* 1..7 */
#define DS3231_REG_DATE          (0x04)  /* 1..31 */
#define DS3231_REG_MONTH         (0x05)  /* bit7 = Centenas */
#define DS3231_REG_YEAR          (0x06)  /* (00..99) */


#ifdef __cplusplus
}
#endif

#endif /* DS3231_REGISTERS_H */