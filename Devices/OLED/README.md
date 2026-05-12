# OLED SSD1315 - Display 0.96" 128×64

## Descripción
Driver para display OLED **SSD1315** de 0.96 pulgadas, resolución 128×64 píxeles.

## Funcionalidad
- Inicialización completa del controlador SSD1315
- Limpieza del display
- Escritura de texto con fuente 6×8 (ASCII 32–127)
- Alternancia automática entre pantalla de pesos y pantalla de reloj cada 3 segundos
- Pantallas predefinidas para los estados del dispensador (idle, dispensando, error)
- Comunicación vía **I²C1** usando el bus compartido con el DS3231

## Disposición de carpetas
```text
Devices/
└── OLED/
    ├── Inc/
    │   ├── dev_oled.h
    │   ├── oled_port.h
    │   └── oled_registers.h
    └── Src/
        ├── dev_oled.c
        └── oled_port.c
```

## Archivos del módulo
| Archivo | Funcionalidad |
|---|---|
| `dev_oled.c` | Lógica de pantallas, buffer interno, fuente 6×8 y API pública. |
| `dev_oled.h` | Tipos públicos (`OLED_Status`, `OLED_Data`) y funciones públicas. |
| `oled_port.c` | Acceso I²C (capa que depende de HAL de STM). |
| `oled_port.h` | Prototipos del port. |
| `oled_registers.h` | Comandos, constantes de configuración y dimensiones del display. |

## Comunicación
- Dirección **I²C1** del SSD1315: `0x3C` (D/C# = GND) / `0x3D` (D/C# = VCC)
- Modo de memoria: **horizontal** — permite volcar el buffer completo en una sola transacción I²C
- Protocolo: `[ADDR] [0x00] [cmd]` para comandos / `[ADDR] [0x40] [datos]` para píxeles

## Pantallas predefinidas

### ST_IDLE — alternan cada 3 segundos

```
PESOS:                      HORA:
┌─────────────────────┐     ┌─────────────────────┐
│   Guardian Pet      │     │   Guardian Pet      │
│─────────────────────│     │─────────────────────│
│ Tanque:             │     │    08:15:30         │
│   4.250 kg          │     │                     │
│ Plato:              │     │  Lun 12/04/25       │
│   15 g              │     │                     │
└─────────────────────┘     └─────────────────────┘
```

### ST_DISPENSING / ST_ERROR

```
DISPENSANDO:                ERROR:
┌─────────────────────┐     ┌─────────────────────┐
│   Guardian Pet      │     │                     │
│─────────────────────│     │      ERROR          │
│   ** DANDO  **      │     │   Reiniciando...    │
│   ** RACION **      │     │                     │
└─────────────────────┘     └─────────────────────┘
```

## API principal
- `OLED_Status OLED_Init(void)`
- `void OLED_Clear(void)`
- `void OLED_PutString(uint8_t col, uint8_t page, const char *str)`
- `void OLED_Update(const OLED_Data *data)`
- `void OLED_ShowDispensing(void)`
- `void OLED_ShowError(void)`

## Ejemplo
```c
if (OLED_Init() == OLED_OK) {
    OLED_Data data = {
        .tankKg     = 4.250f,
        .plateGrams = 15.0f,
        .hour       = 8,
        .minute     = 15,
        .second     = 30,
        .day        = 12,
        .month      = 4,
        .year       = 25,
        .weekday    = 1,  // Lun
    };
    OLED_Update(&data);      // ST_IDLE — alterna pesos/hora automáticamente
    OLED_ShowDispensing();   // ST_DISPENSING
    OLED_ShowError();        // ST_ERROR
}
```

## Notas
- El buffer interno ocupa **1025 bytes** en RAM (1 byte de control + 1024 bytes de píxeles).
- La fuente 6×8 permite hasta **21 caracteres por línea** y **8 líneas** (páginas).
- `col` se expresa en píxeles (0–127); `page` en filas de 8px (0–7).
- `OLED_Update` debe llamarse periódicamente desde `SM_Task` en `ST_IDLE`.
