# HX711 - Amplificador para Galga Extensiométrica

## Descripción
Driver para dos módulos **HX711** conectados a celdas de carga — una para el **tanque** (3/5 kg) y otra para el **plato** (1 kg).

## Funcionalidad
- Inicialización y activación de ambos módulos HX711
- Lectura de peso en gramos con promediado de muestras para reducir ruido
- Tara individual o simultánea de ambas galgas
- Ajuste del factor de escala para calibración
- Modo power-down y power-up para bajo consumo
- Comunicación por **GPIO bit-bang** (sin periférico SPI/I²C)

## Disposición de carpetas
```text
Devices/
└── HX711/
    ├── Inc/
    │   ├── dev_hx711.h
    │   ├── hx711_port.h
    │   └── hx711_registers.h
    └── Src/
        ├── dev_hx711.c
        └── hx711_port.c
```

## Archivos del módulo
| Archivo | Funcionalidad |
|---|---|
| `dev_hx711.c` | Lógica de lectura, tara, calibración y API pública. |
| `dev_hx711.h` | Tipos públicos (`HX711_Status`) y funciones públicas. |
| `hx711_port.c` | Bit-bang GPIO: pulsos SCK y lectura DOUT (capa HAL). |
| `hx711_port.h` | Prototipos del port. |
| `hx711_registers.h` | Pines, canales, timeouts y factores de calibración por defecto. |

## Conexiones
| Pin STM32 | HX711 Tanque | HX711 Plato | Función |
|---|---|---|---|
| PC0 | DOUT | — | Dato serie tanque |
| PC1 | SCK | — | Clock tanque |
| PC2 | — | DOUT | Dato serie plato |
| PC3 | — | SCK | Clock plato |

## Protocolo
- Canal A, Ganancia 128 → **25 pulsos SCK** por lectura
- Los primeros 24 pulsos transfieren los datos (MSB first)
- El pulso 25 selecciona Canal A Gain 128 para la próxima lectura
- DOUT = LOW indica dato listo (~100 ms entre muestras a 10 Hz)
- Timeout de espera: **500 ms**

## Canales
| Identificador | Canal | Celda | Rango |
|---|---|---|---|
| `HX711_CH_TANK` | 0 | Tanque | 5 kg |
| `HX711_CH_PLATE` | 1 | Plato | 5 kg |

## Calibración
Cada canal tiene un factor de escala (crudo/gramo) ajustable en tiempo de ejecución:

| Canal | Factor por defecto |
|---|---|
| Tanque | 420.0 |
| Plato | 410.0 |

Procedimiento:
1. Llamar `HX711_TareAll()` con las celdas en vacío
2. Colocar un peso conocido
3. Leer el valor crudo promediado
4. `scale = (crudo - offset) / peso_gramos`
5. Llamar `HX711_SetScale(ch, scale)`

## API principal
- `HX711_Status HX711_Init(void)`
- `HX711_Status HX711_ReadGrams(uint8_t ch, float *grams)`
- `HX711_Status HX711_Tare(uint8_t ch)`
- `HX711_Status HX711_TareAll(void)`
- `void HX711_SetScale(uint8_t ch, float scale)`
- `void HX711_PowerDown(void)`
- `void HX711_PowerUp(void)`

## Ejemplo
```c
if (HX711_Init() == HX711_OK) {
    HX711_TareAll();  // tarar ambas galgas en vacío

    float tankGrams  = 0.0f;
    float plateGrams = 0.0f;

    if (HX711_ReadGrams(HX711_CH_TANK,  &tankGrams)  == HX711_OK &&
        HX711_ReadGrams(HX711_CH_PLATE, &plateGrams) == HX711_OK) {
        // usar tankGrams y plateGrams...
    }
}
```

## Notas
- `HX711_ReadGrams` es **bloqueante** (~800 ms con 8 muestras promediadas). Llamar desde una tarea dedicada o desde `SM_Task` en `ST_IDLE`.
- El delay de espera dentro del loop de `HX711_port_wait_ready` usa `osDelay` para ceder el CPU al scheduler durante la espera.
- SCK en HIGH por más de 60 µs pone el HX711 en power-down. Usar `HX711_PowerDown` / `HX711_PowerUp` para bajo consumo.
