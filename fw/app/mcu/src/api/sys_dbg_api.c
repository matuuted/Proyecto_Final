# include "api.h"
# include "sys_dbg_api.h"

const FPTRS SYS_DBG_FPTRS[SYS_DBG_FUNCTIONS_COUNT] = {

    { .module_id = SYS_DBG_API_ID,
      .function_id = DBG_INITOS,
      .task_id = NONE_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) DBG_InitOS },
    },

    { .module_id = SYS_DBG_API_ID,
      .function_id = DBG_ITER,
      .task_id = NONE_TASK_ID,
      .nargs = 0,
      .arg_types = { 0 },
      .arg_lens = { 0 },
      .fptr = { .f0 = (fptr_0) DBG_Iter },
    },
};