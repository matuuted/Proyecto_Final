/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : api.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __API_H
#define __API_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>

#define REG_STATUS       0
#define REG_MODULE_ID    1
#define REG_FUNCTION_ID  2
#define REG_ARG_TYPES    3
#define REG_DATA_START   4

#define API_GO           0x2
#define API_BUSY         0x1

#define ARGTYPE_U32      0
#define ARGTYPE_TLV      1

#define API_ALIGN(x)    ( ((x)+3) & 0xFFFC )
#define STRUCT_COUNT(x) ( sizeof(x) / sizeof(x[0]) )

#define ARGTYPE_MASK     0x7F

#define TLV_TYPE         0
#define TLV_LENGTH       1
#define TLV_VALUE        4
#define TLV_HDRLEN       4

#define ARGTYPE_PTR      0x80

#define ARGLEN_PARAM    0x8000

typedef enum TASK_T {
  NONE_TASK_ID  = 0,
  ANY_TASK_ID   = 1,
} TASK_TYPE;

typedef enum {
    UINT8_T,   // equivalente a uint8_t
    INT8_T,    // equivalente a int8_t
    UINT16_T,  // equivalente a uint16_t
    INT16_T,   // equivalente a int16_t
    UINT32_T,  // equivalente a uint32_t
    INT32_T,   // equivalente a int32_t
    FLOAT32_T, // equivalente a float (32-bit IEEE-754)
    FLOAT16_T, // equivalente a _Float16 / __fp16 (si lo soporta el compilador)
    UINT64_T,  // equivalente a uint64_t
    __VOID     // equivalente a void
} ARG_TYPE;

#define MODULE_GROUP_SHIFT  6
#define MODULE_GROUP_MASK     ( (0xFF << MODULE_GROUP_SHIFT) & 0xFF )
#define MODULE_GROUP(m)       ( (m) >> MODULE_GROUP_SHIFT ) 
#define MODULE_ISGROUP(m, g)  ( (m) & MODULE_GROUP_MASK == (g) )
#define MODULE_INDEX(m)       ( (m) & ~MODULE_GROUP_MASK )
#define MODULE_GROUPS        4
#define NONE_modules         0xff

typedef HAL_StatusTypeDef (*fptr_0) (void);
typedef HAL_StatusTypeDef (*fptr_1) (uintptr_t arg0);
typedef HAL_StatusTypeDef (*fptr_2) (uintptr_t arg0, uintptr_t arg1);
typedef HAL_StatusTypeDef (*fptr_3) (uintptr_t arg0, uintptr_t arg1, uintptr_t arg2);
typedef HAL_StatusTypeDef (*fptr_4) (uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3);
typedef HAL_StatusTypeDef (*fptr_5) (uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);

#define MAX_ARGS 8

typedef struct FPTRS_T {
    uint8_t module_id;            /**< Module ID */
    uint8_t function_id;          /**< Function ID within module */
    uint8_t task_id;              /**< Task ID to use to run the function */
    uint8_t nargs;                /**< Number of arguments to function */
    uint8_t arg_types[MAX_ARGS];  /**< Argument types (from ARGTYPE), including ARGTYPE_PTR if appropriate */
    uint16_t arg_lens[MAX_ARGS];  /**< Length of pointer arguments, including ARGLEN_PARAM if appropriate */

    union {
        fptr_0 f0;
        fptr_1 f1;
        fptr_2 f2;
        fptr_3 f3;
        fptr_4 f4;
        fptr_5 f5;
    } fptr;
} FPTRS;


#define COMMAND_SIZE 1280

typedef struct {
  uint8_t buffer[COMMAND_SIZE];
} COMMAND_DATA __attribute__((aligned(4)));

extern COMMAND_DATA command;

extern void exec_fn(uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __API_H */
