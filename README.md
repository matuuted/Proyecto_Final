# Proyecto Final – Medición de Inclinación

## Descripción
Este proyecto utiliza la placa **NUCLEO-STM32F446RE** y permite detectar y visualizar en tiempo real la **inclinación** mediante el sensor **MPU-6050**.  
La información del ángulo se muestra en un display **LCD 16x2** y se envía por **UART**, junto con la **hora actual del RTC DS3231**.
Además, esta inclinación se ve reflejada en el **LED** con una frecuencia de parpadeo correspondiente al ángulo medido.  

---

## Hardware Utilizado
- **Placa principal:** STM32 Nucleo-F446RE  
- **Sensor de movimiento:** MPU-6050 
- **Reloj en tiempo real:** DS3231  
- **Display:** LCD 16×2 (I²C)  
- **Comunicación:** UART + I²C  
- **LED:** integrado en la placa Nucleo  

---

## Funcionamiento General
1. **Inicialización:**  
   Se configuran los periféricos (I²C, UART, GPIO) y se verifica la presencia del LCD, MPU-6050 y DS3231.  

2. **Lectura de Datos:**  
   Se obtienen los valores de aceleración desde el MPU-6050.  

3. **Procesamiento:**  
   Se aplica un filtro simple para calcular el ángulo, que luego se vera representado en el parpadeo del LED.  

4. **Visualización:**  
   - **LCD 16x2:** muestra los valores de inclinación.  
   - **UART:** envía una línea con hora + datos del sensor.  
   - **LED:** parpadea a una frecuencia dependendiendo el ángulo detectado.

---

## Módulos de Software
| Módulo            | Función Principal                           |
|-------------------|---------------------------------------------|
| `mpu6050.c / .h`  | Lectura y configuración del sensor MPU-6050 |
| `ds3231.c / .h`   | Manejo del reloj RTC DS3231                 |
| `dev_uart.c / .h` | Envio de Datos a traves de la UART          |
| `dev_LCD.c / .h`  | Control del display LCD 16×2                |
| `sys_sm.c / .h`   | Máquina de estados principal                |

---

## Documentación

La documentación del proyecto final, la cual fué generada con **Doxygen**, se encuentra disponible en el siguiente enlace:

- [Ver documentación](https://matuuted.github.io/Proyecto_Final/)

---

##  Autor
**Matías Durante**  
Proyecto Final
Año: **2025**
