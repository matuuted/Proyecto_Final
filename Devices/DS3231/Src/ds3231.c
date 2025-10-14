/**
 * @file    ds3231.c
 * @brief   Implementación del driver DS3231, utilizando ds3231_port.c.
 */

#include "ds3231.h"
#include "ds3231_registers.h"
#include "ds3231_port.h"

/* -------------------------------------------------------------------------- */
/*  API pública                                                               */
/* -------------------------------------------------------------------------- */

static DS3231_Status DS3231_parse_hal_status(HAL_StatusTypeDef hal_status)
{
    switch (hal_status)
    {
        case HAL_OK:      return DS3231_OK;
        case HAL_TIMEOUT: return DS3231_TIMEOUT;
        case HAL_ERROR:   return DS3231_ERROR;
        case HAL_BUSY:    return DS3231_BUSY;
        default:          return DS3231_ERROR;
    }
}

DS3231_Status DS3231_Init(void)
{
    return DS3231_parse_hal_status(DS3231_is_ready());
}


DS3231_Status DS3231_ReadTime(DS3231_Time *time)
{
    if (!time) return DS3231_INVALID_PARAM;

    DS3231_Status status = DS3231_OK;
    uint8_t buf[DS3231_MAX_BLOCK_READ];

    status = DS3231_parse_hal_status(DS3231_register_block_read(DS3231_REG_SECONDS, buf,  sizeof(buf)));
    if (status != DS3231_OK) return status;

    time->seconds = (uint8_t)bcd2dec(buf[0] & 0x7F);
    time->minutes = (uint8_t)bcd2dec(buf[1] & 0x7F);
    time->hours   = (uint8_t)bcd2dec(buf[2] & 0x3F);
    time->day     = (uint8_t)(buf[3] & 0x07);       // El dia no hace falta convertirlo, porque es el dia semanal (1...7)
    time->date    = (uint8_t)bcd2dec(buf[4] & 0x3F);
    time->month   = bcd2dec(buf[5] & 0x1F);         // Enmascaro ya que el bit7 es el centenario.
    time->year    = bcd2dec(buf[6]);

    return status;
}

static DS3231_Status DS3231_SetTimeBCD(const DS3231_Time *time)
{
    if (!time) return DS3231_INVALID_PARAM;

    DS3231_Status status = DS3231_OK;
    uint8_t buf[DS3231_MAX_BLOCK_WRITE];

    buf[0] = DS3231_REG_SECONDS;
    buf[1] = (uint8_t)(time->seconds & 0x7F);
    buf[2] = (uint8_t)(time->minutes & 0x7F);
    buf[3] = (uint8_t)(time->hours   & 0x3F);
    buf[4] = (uint8_t)(time->day     & 0x07);
    buf[5] = (uint8_t)(time->date    & 0x3F);
    buf[6] = time->month;                     
    buf[7] = time->year;

    status = DS3231_parse_hal_status(DS3231_register_block_write(buf,  sizeof(buf)));
    return status;
}

DS3231_Status DS3231_SetTime(uint8_t year, uint8_t month, uint8_t date,
                                    uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    if ((sec > 59 || min > 59 || hour > 23) || 
        (day < 1  || day > 7) ||
        (date < 1 || date > 31) || 
        (month < 1 || month > 12) || 
        year > 99)
    {
        return DS3231_INVALID_PARAM;
    }

    DS3231_Time time;
    time.seconds = dec2bcd(sec);
    time.minutes = dec2bcd(min);
    time.hours   = dec2bcd(hour);
    time.day     = (uint8_t)(day & 0x07);
    time.date    = dec2bcd(date);
    time.month   = dec2bcd(month);
    time.year    = dec2bcd(year);

    return DS3231_SetTimeBCD(&time);
}