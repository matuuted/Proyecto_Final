/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : api.c
  * Description        : Code for api applications
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
#include "api.h"
#include "dev_modules.h"

const uint8_t MODULE_INDEX_COUNT[] = {
    DEV_MODULES_COUNT,
    // SYS_MODULES_COUNT,
    // APP_MODULES_COUNT,
};

const FPTRS **FPTRS_LIST[] = {
    FPTRS_LIST_DEV,
    // FPTRS_LIST_SYS,
    // FPTRS_LIST_APP,
};

const uint8_t *FPTRS_COUNT[] = {
    FPTRS_COUNT_DEV,
    // FPTRS_COUNT_SYS,
    // FPTRS_COUNT_APP,
};

const FPTRS *find_fn(uint8_t m, uint8_t f)
{
    // first check if module is valid
    if (m == NONE_modules) return NULL; // invalid
    int g = MODULE_GROUP(m);
    if (g > MODULE_GROUPS) return NULL;

    int i = MODULE_INDEX(m);
    if (i > MODULE_INDEX_COUNT[g]) return NULL;

    const FPTRS *list = FPTRS_LIST[g][i];

    if (f < FPTRS_COUNT[g][i]) {
        if (list[f].module_id == m && list[f].function_id == f) {
            return &list[f];
        }
    }

    return NULL;
}


void exec_fn(uint8_t *data)
{
    HAL_StatusTypeDef retVal = HAL_ERROR;

    if (data[REG_STATUS] != API_GO) return;

    data[REG_STATUS] = API_BUSY;

    const FPTRS *f = find_fn(data[REG_MODULE_ID], data[REG_FUNCTION_ID]);
    if (f != NULL) {
        uint8_t argTypes = data[REG_ARG_TYPES];
        uintptr_t args[8] = { 0 };
        int arg_offset = REG_DATA_START;
        int arg_id;

        for (arg_id = 0; arg_id < f->nargs; arg_id++) {
            if ((argTypes & 1) == ARGTYPE_U32) {
                arg_offset = API_ALIGN(arg_offset);
                args[arg_id] = (uintptr_t)*(uint32_t *) & data[arg_offset];
                arg_offset += 4;
            } else {
            	uint32_t *p = (uint32_t *)&data[arg_offset + TLV_VALUE];
                if (data[arg_offset + TLV_TYPE] & ARGTYPE_PTR){
                    args[arg_id] = (uintptr_t) p;
                }else {
                  switch (data[arg_offset + TLV_TYPE] & ARGTYPE_MASK) {
                      case UINT8_T:
                      case INT8_T:
                          args[arg_id] = (uintptr_t)(uint8_t) *p;
                          break;
  
                      case UINT16_T:
                      case INT16_T:
                      case FLOAT16_T:
                          args[arg_id] = (uintptr_t)(uint16_t) *p;
                          break;
  
                      case UINT32_T:
                      case INT32_T:
                      case FLOAT32_T:
                          args[arg_id] = (uintptr_t) *p;
                          break;
  
                      default:
                          data[REG_STATUS] = (uint8_t)HAL_ERROR;
                          return;
                  }
                }
                arg_offset += *(uint16_t *)&data[arg_offset + TLV_LENGTH];
			}
			argTypes >>= 1;
        }
        switch (f->nargs) {
			default:
				break;
            case 0:
                retVal = f->fptr.f0();
                break;

            case 1:
                retVal = f->fptr.f1(args[0]);
                break;

            case 2:
                retVal = f->fptr.f2(args[0], args[1]);
                break;

            case 3:
                retVal = f->fptr.f3(args[0], args[1], args[2]);
                break;

            case 4:
                retVal = f->fptr.f4(args[0], args[1], args[2], args[3]);
                break;

            case 5:
                retVal = f->fptr.f5(args[0], args[1], args[2], args[3], args[4]);
                break;
        }
    }
    data[REG_STATUS] = (uint8_t)retVal;
}