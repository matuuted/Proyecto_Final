# NEMA 17 - Motor Paso a Paso vía A4988

## Descripción
Driver para motor paso a paso **NEMA 17** (1.8°/paso) controlado mediante el driver **A4988** y PWM por **TIM2**.

## Funcionalidad
- Inicialización de GPIO y TIM2 en modo PWM
- Giro no bloqueante por cantidad fija de pasos
- Control de dirección (horario / antihorario)
- Ajuste de velocidad en pasos/segundo en tiempo de ejecución
- Conteo de pasos por IRQ del TIM2 (Update Event) — independiente del jitter del RTOS
- Detención inmediata con deshabilitación de bobinas

## Disposición de carpetas
```text
Devices/
└── NEMA17/
    ├── Inc/
    │   ├── dev_motor.h
    │   ├── motor_port.h
    │   └── motor_registers.h
    └── Src/
        ├── dev_motor.c
        └── motor_port.c
```

## Archivos del módulo
| Archivo | Funcionalidad |
|---|---|
| `dev_motor.c` | Lógica de pasos, dirección, estado y API pública. |
| `dev_motor.h` | Tipos públicos (`Motor_Status`, `Motor_Dir`) y funciones públicas. |
| `motor_port.c` | Configuración de GPIO, TIM2 PWM e IRQ (capa HAL). |
| `motor_port.h` | Prototipos del port. |
| `motor_registers.h` | Pines, timer, velocidades y constantes mecánicas. |

## Conexiones
| Pin STM32 | A4988 | Función |
|---|---|---|
| PA5 (TIM2_CH1) | STEP | Pulso de paso (PWM) |
| PA6 | DIR | Dirección de giro |
| PA7 | ENABLE | Habilitación (LOW = activo) |
| VCC | SLEEP / RESET | Siempre activo |
| GND | MS1 / MS2 / MS3 | Paso completo (200 pasos/rev) |

## Timer
- **TIM2 CH1** genera el PWM en PA5 (STEP)
- APB1 = 42 MHz → TIM2 CLK = 84 MHz
- Prescaler = 83 → CLK efectivo = **1 MHz**
- ARR = 1 MHz / SPS − 1 → frecuencia = pasos por segundo
- Cada **Update Event** = 1 paso completado → contado en IRQ

## Velocidades
| Parámetro | Valor |
|---|---|
| Velocidad por defecto | 400 SPS (2 rev/seg) |
| Velocidad mínima | 50 SPS |
| Velocidad máxima | 2000 SPS |
| Pasos por revolución | 200 (paso completo) |
| Pasos por ración estándar | 400 (2 vueltas) |

## API principal
- `Motor_Status Motor_Init(void)`
- `void Motor_Dispense(uint32_t steps, Motor_Dir dir)`
- `void Motor_Stop(void)`
- `bool Motor_IsDone(void)`
- `void Motor_SetSpeed(uint32_t sps)`
- `void Motor_TIM2_IRQHandler(void)` — llamar desde `TIM2_IRQHandler` en `stm32f4xx_it.c`

## Ejemplo
```c
if (Motor_Init() == MOTOR_OK) {
    Motor_SetSpeed(400);                          // 400 pasos/seg
    Motor_Dispense(400, MOTOR_DIR_CW);            // 2 vueltas horario, no bloqueante

    while (!Motor_IsDone()) {
        osDelay(10);                              // ceder al scheduler
    }

    // Motor detenido y bobinas deshabilitadas automáticamente
}
```

## Notas
- `Motor_Dispense` es **no bloqueante** — el motor se detiene automáticamente al completar los pasos via IRQ.
- Al finalizar los pasos, el A4988 se deshabilita (ENABLE = HIGH) para evitar calentamiento de bobinas.
- Llamar `Motor_TIM2_IRQHandler()` desde `TIM2_IRQHandler()` en `stm32f4xx_it.c`.
- No llamar `Motor_SetSpeed` mientras el motor está en movimiento.
