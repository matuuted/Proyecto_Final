# include "api.h"
# include "dev_i2cs_api.h"

const FPTRS DEV_I2CS_FPTRS[DEV_I2CS_FUNCTIONS_COUNT] = {

    { .module_id = DEV_I2CS_API_ID,
      .function_id = I2CS_INITSTART,
      .task_id = ANY_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) I2CS_InitStart },
    },

    { .module_id = DEV_I2CS_API_ID,
      .function_id = I2CS_DEINIT,
      .task_id = ANY_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) I2CS_DeInit },
    },

    { .module_id = DEV_I2CS_API_ID,
      .function_id = I2C2_ENABLEACKNOWLEDGE,
      .task_id = ANY_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) I2C2_EnableAcknowledge },
    },

    { .module_id = DEV_I2CS_API_ID,
      .function_id = I2C2_DISABLEACKNOWLEDGE,
      .task_id = ANY_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) I2C2_DisableAcknowledge },
    },
};