#include "sys_modules.h"
#include "sys_dbg_api.h"

const FPTRS *FPTRS_LIST_SYS[SYS_MODULES_COUNT] =
{
        [MODULE_INDEX(SYS_DBG_API_ID)] = SYS_DBG_FPTRS,
};

const uint8_t FPTRS_COUNT_SYS[SYS_MODULES_COUNT] = 
{
        [MODULE_INDEX(SYS_DBG_API_ID)] = STRUCT_COUNT(SYS_DBG_FPTRS),
};
