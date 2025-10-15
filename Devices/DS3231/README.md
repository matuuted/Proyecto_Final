# DS3231 - Reloj en Tiempo Real (RTC)

## Descripción
Driver para el reloj en tiempo real **DS3231**. 

## Funcionalidad
- Permite inicializar el dispositivo
- Leer la hora/fecha actual y configurar el RTC a través de **I²C**.

## Disposición de carpetas
```text
Devices/
└── DS3231/
    ├── Inc/
    │   ├── ds3231.h
    │   ├── ds3231_port.h
    │   └── ds3231_registers.h
    └── Src/
        ├── ds3231.c
        └── ds3231_port.c
```

## Archivos del módulo
| Archivo | Funcionalidad |
|---|---|
| `ds3231.c` | Implementación de lectura/escritura de hora/fecha. |
| `ds3231.h` | Tipos públicos (`DS3231_Time`, `DS3231_Status`) y funciones públicas. |
| `ds3231_port.c` | Acceso I²C (capa que depende de HAL de STM). |
| `ds3231_port.h` | Prototipos del port. |
| `ds3231_registers.h` | Direcciones y máscaras de registros usados. |

## Comunicación
- Dirección **I²C1** del DS3231: `0x68`

## API principal
- `DS3231_Status DS3231_Init(void)`
- `DS3231_Status DS3231_ReadTime(DS3231_Time *time)`
- `DS3231_Status DS3231_SetTime(uint8_t yy, uint8_t mm, uint8_t dd, uint8_t dow, uint8_t hh, uint8_t mi, uint8_t ss)`

## Ejemplo
```c
if (DS3231_Init() == DS3231_OK) {
    (void)DS3231_SetTime(25, 10, 14, 2, 12, 30, 0); // 2025-10-14 12:30:00
    DS3231_Time now;
    if (DS3231_ReadTime(&now) == DS3231_OK) {
        // usar now.hours, now.minutes, now.seconds...
    }
}
```
