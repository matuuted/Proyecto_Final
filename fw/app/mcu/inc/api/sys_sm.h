#ifndef SYS_SM_API_H
#define SYS_SM_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"  // por HAL_StatusTypeDef

/* -------------------------------------------------------------------------- */
/*                         Estados de la SM                                   */
/* -------------------------------------------------------------------------- */
typedef enum {
    ST_INIT = 0,
    ST_READ_SENSORS,
    ST_UPDATE_LED,
    ST_UPDATE_UART,
    ST_ERROR,
} SM_State;

/* -------------------------------------------------------------------------- */
/*                                API pública                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief Crea e inicializa la tarea principal de la máquina de estados, utilizando el FreeRTOS proporcionado por el CubeMX.
 * @retval HAL_OK si la tarea fue creada correctamente.
 */
HAL_StatusTypeDef SM_InitOS(void);

/**
 * @brief Tarea que ejecuta la máquina de estados.
 */
void SM_Iter(void *argument);


#ifdef __cplusplus
}
#endif

#endif /* SYS_SM_API_H */
