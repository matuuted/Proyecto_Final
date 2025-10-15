# LCD 16x2 — Driver HD44780 vía I²C (PCF8574)

## Descripción
Driver para pantalla **LCD 16×2** con controlador **HD44780** utilizando un driver **PCF8574** (I²C).

- El módulo contiene inicialización del display en modo 4‑bit, funciones de escritura de texto,
posicionamiento de cursor, limpieza y centrado.

## Disposición de carpetas
```text
Devices/
└── LCD16x2/
    ├── Inc/
    │   ├── dev_LCD.h
    │   ├── LCD_port.h
    │   └── LCD_registers.h
    └── Src/
        ├── dev_LCD.c
        └── LCD_port.c
```

## Archivos del módulo
| Archivo | Funcionalidad |
|---|---|
| `dev_LCD.c` | Funciones para la inicialización del display y escritura de datos. |
| `dev_LCD.h` | Prototipos y constantes públicas. |
| `LCD_port.c` | Escritura de registros (dependiente de HAL). |
| `LCD_port.h` | Prototipos del port. |
| `LCD_registers.h` | Comandos y flags del **HD44780/PCF8574** utilizados en **dev_LCD.c**. |

## Comunicación
- Dirección del módulo: `0x27`

## API principal
- `bool LCD_Init(void)`
- `void LCD_Write(const char *s)` / `void LCD_WriteAt(uint8_t col, uint8_t row, char *s)`
- `void LCD_Clear(void)` / `void LCD_Cursor(bool on)` / `void LCD_BlinkCursor(bool on)`

## Ejemplo
```c
if (LCD_Init()) {
    LCD_PrintCentered(0, "Funcionando...");
    LCD_WriteAt(0, 1, "LCD OK");
}
```
