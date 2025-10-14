from api import *

def bcd2dec(x: int) -> int:
    """Convierte BCD a decimal."""
    return ((x >> 4) * 10) + (x & 0x0F)

def dec2bcd(x: int) -> int:
    """Convierte decimal a BCD."""
    return ((x // 10) << 4) | (x % 10)

def ds3231_print_datetime(status: int, raw: list[int]):
    if status != 0:
        print(f"Error leyendo DS3231: status={status}")
        return

    if len(raw) < 7:
        print("Datos incompletos")
        return

    sec   = bcd2dec(raw[0] & 0x7F)
    minute= bcd2dec(raw[1] & 0x7F)
    hour  = bcd2dec(raw[2] & 0x3F)  # 24h
    dow   = raw[3] & 0x07
    date  = bcd2dec(raw[4] & 0x3F)
    month = bcd2dec(raw[5] & 0x1F)
    year  = 2000 + bcd2dec(raw[6])  # Century=0

    dias = {1:"Dom",2:"Lun",3:"Mar",4:"Mié",5:"Jue",6:"Vie",7:"Sáb"}

    print(f"{dias.get(dow,'?')} {date:02d}/{month:02d}/{year} "
        f"{hour:02d}:{minute:02d}:{sec:02d}")

def ds3231_get_time():
    status, raw = i2cm.I2CM_Read_Sr(0x68,0,7)
    ds3231_print_datetime(status, raw)

def ds3231_set_time(year: int, month: int, date: int, dow: int, hour: int, minute: int, sec: int):
    """Establece la fecha y hora del DS3231.
    year: año (ej. 2024)
    month: mes (1-12)
    date: día del mes (1-31)
    dow: día de la semana (1=Dom, 2=Lun, ..., 7=Sáb)
    hour: hora (0-23)
    minute: minuto (0-59)
    sec: segundo (0-59)
    """

    if year < 2000 or year > 2099:
        raise ValueError("Año fuera de rango (2000-2099)")
    if month < 1 or month > 12:
        raise ValueError("Mes fuera de rango (1-12)")
    if date < 1 or date > 31:
        raise ValueError("Día fuera de rango (1-31)")
    if dow < 1 or dow > 7:
        raise ValueError("Día de la semana fuera de rango (1-7)")
    if hour < 0 or hour > 23:
        raise ValueError("Hora fuera de rango (0-23)")
    if minute < 0 or minute > 59:
        raise ValueError("Minuto fuera de rango (0-59)")
    if sec < 0 or sec > 59:
        raise ValueError("Segundo fuera de rango (0-59)")

    year_bcd = dec2bcd(year - 2000)
    month_bcd = dec2bcd(month)
    date_bcd = dec2bcd(date)
    dow_bcd = dow & 0x07
    hour_bcd = dec2bcd(hour)
    minute_bcd = dec2bcd(minute)
    sec_bcd = dec2bcd(sec)

    SEC_REG = 0x00
    data = [SEC_REG,sec_bcd, minute_bcd, hour_bcd, dow_bcd, date_bcd, month_bcd, year_bcd]
    status = i2cm.I2CM_Write(0x68, data)
    if status != 0:
        print(f"Error estableciendo hora en DS3231): status={status}")