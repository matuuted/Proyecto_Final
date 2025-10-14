#pragma once
#include <math.h>

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / 3.14159265358979323846f)
#endif

/* ====== Cálculo de inclinación ======
 * Fórmula: atan2f(-Ax, sqrt(Ay^2 + Az^2)) * RAD_TO_DEG
 * - Devuelve el ángulo de inclinación respecto del eje X (en grados).
 * - ax, ay, az deben estar expresados en "g" (aceleración normalizada).
 */
static inline float mpu_get_angle_deg(float ax, float ay, float az)
{
    return atanf(-ax / sqrtf(ay*ay + az*az)) * RAD_TO_DEG;
}
