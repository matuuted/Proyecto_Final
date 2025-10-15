# MPU6050 — Driver IMU (acelerómetro + giroscopio)

## Descripción
Driver para el sensor **MPU6050**. 
- Permite inicializar el dispositivo y leer los datos de aceleración y giro. 

## Disposición de carpetas
```text
Devices/
└── MPU6050/
    ├── Inc/
    │   ├── mpu6050.h
    │   ├── mpu6050_port.h
    │   ├── mpu6050_registers.h
    │   └── mpu_math.h
    └── Src/
        ├── mpu6050.c
        └── mpu6050_port.c
```

## Archivos del módulo
| Archivo | Funcionalidaad |
|---|---|
| `mpu6050.c` | Inicialización, lectura de los ejes (X, Y y Z) y conversión del ángulo a grados. |
| `mpu6050.h` | Tipos públicos (`MPU6050_Handler`, `MPU6050_Data`) y prototipos. |
| `mpu6050_port.c` | Acceso I²C al sensor (depende de HAL). |
| `mpu6050_port.h` | Prototipos del port. |
| `mpu6050_registers.h` | Registros y máscaras usados por el driver. |
| `mpu_math.h` | Cálculo de ángulo. |

## Comunicación
- Dirección del dispositivo: `0x69`

## API principal
- `MPU6050_Status MPU6050_Init(MPU6050_Handler *dev)`
- `MPU6050_Status MPU6050_Read(MPU6050_Handler *dev)`
- `MPU6050_Status MPU6050_ReadRaw(MPU6050_DataRaw *raw)`

## Ejemplo
```c
MPU6050_Handler mpu;
if (MPU6050_Init(&mpu) == MPU6050_OK) {
    if (MPU6050_Read(&mpu) == MPU6050_OK) {
        float ax = mpu.data.accel_x;
        float gz = mpu.data.gyro_z;
    }
}
```
