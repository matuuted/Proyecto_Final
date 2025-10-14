
#ifndef SYS_DBG_API_H
#define SYS_DBG_API_H

#include "api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

#define SYS_DBG_API_ID 64
#define OS_NOTIFY_I2C_COMMAND (1 << 0)

extern osThreadId_t DebugTaskHandle;


/* API Identifiers */
enum SYS_DBG_API {
    DBG_INITOS = 0,
    DBG_ITER = 1,
};

/* API Function List */
#define SYS_DBG_FUNCTIONS_COUNT 2
extern const FPTRS SYS_DBG_FPTRS[SYS_DBG_FUNCTIONS_COUNT];


/**
*   Initializes the RTOS kernel and related components.
*
* \return HAL_StatusTypeDef
*
*   RTOS Info: NONE
**/
HAL_StatusTypeDef DBG_InitOS (void);

 

/**
*   Performs the iteration of the debug task.
*
* \return void
*
*   RTOS Info: NONE
**/
void DBG_Iter (void *argument);

 
 

#endif // _SYS__API_H