/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "sys_dbg_api.h"

COMMAND_DATA command = { .buffer = {0} };

osThreadId_t DebugTaskHandle;
const osThreadAttr_t DebugTask_attributes = {
  .name = "DebugTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

HAL_StatusTypeDef DBG_InitOS(void)
{
    DebugTaskHandle = osThreadNew(DBG_Iter, NULL, &DebugTask_attributes);
    if (DebugTaskHandle == NULL) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

void DBG_Iter(void *argument)
{
    for (;;) {
        uint32_t f = osThreadFlagsWait(OS_NOTIFY_I2C_COMMAND, osFlagsWaitAny, osWaitForever);

        if ((int32_t)f < 0) continue;

        osThreadFlagsClear(OS_NOTIFY_I2C_COMMAND);

        /* Procesar el comando y ejecutar la funcion. */
        exec_fn(command.buffer);
    }
}
