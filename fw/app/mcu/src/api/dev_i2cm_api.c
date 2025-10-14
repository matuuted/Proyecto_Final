# include "api.h"
# include "dev_i2cm_api.h"

const FPTRS DEV_I2CM_FPTRS[DEV_I2CM_FUNCTIONS_COUNT] = {

    { .module_id = DEV_I2CM_API_ID,
      .function_id = I2CM_INITSTART,
      .task_id = ANY_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) I2CM_InitStart },
    },

    { .module_id = DEV_I2CM_API_ID,
      .function_id = I2CM_DEINIT,
      .task_id = ANY_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) I2CM_DeInit },
    },

    { .module_id = DEV_I2CM_API_ID,
      .function_id = I2CM_WRITE,
      .task_id = ANY_TASK_ID,
      .nargs = 3,
      .arg_types = { UINT8_T, UINT8_T | ARGTYPE_PTR, UINT16_T },
      .arg_lens = { 0, 2 | ARGLEN_PARAM, 0 },
      .fptr = { .f3 = (fptr_3) I2CM_Write },
    },

    { .module_id = DEV_I2CM_API_ID,
      .function_id = I2CM_READ,
      .task_id = ANY_TASK_ID,
      .nargs = 3,
      .arg_types = { UINT8_T, UINT8_T | ARGTYPE_PTR, UINT16_T },
      .arg_lens = { 0, 2 | ARGLEN_PARAM, 0 },
      .fptr = { .f3 = (fptr_3) I2CM_Read },
    },

    { .module_id = DEV_I2CM_API_ID,
      .function_id = I2CM_READ_SR,
      .task_id = ANY_TASK_ID,
      .nargs = 4,
      .arg_types = { UINT8_T, UINT8_T, UINT8_T | ARGTYPE_PTR, UINT16_T },
      .arg_lens = { 0, 0, 3 | ARGLEN_PARAM, 0 },
      .fptr = { .f4 = (fptr_4) I2CM_Read_Sr },
    },

    { .module_id = DEV_I2CM_API_ID,
      .function_id = I2CM_ISDEVICEREADY,
      .task_id = ANY_TASK_ID,
      .nargs = 2,
      .arg_types = { UINT8_T, UINT32_T },
      .arg_lens = { 0, 0 },
      .fptr = { .f2 = (fptr_2) I2CM_IsDeviceReady },
    },
};