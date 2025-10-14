#include "dev_modules.h"
#include "dev_i2cs_api.h"
#include "dev_i2cm_api.h"

const FPTRS *FPTRS_LIST_DEV[DEV_MODULES_COUNT] =
{
        [MODULE_INDEX(DEV_I2CS_API_ID)] = DEV_I2CS_FPTRS,
        [MODULE_INDEX(DEV_I2CM_API_ID)] = DEV_I2CM_FPTRS,
};

const uint8_t FPTRS_COUNT_DEV[DEV_MODULES_COUNT] = 
{
        [MODULE_INDEX(DEV_I2CS_API_ID)] = STRUCT_COUNT(DEV_I2CS_FPTRS),
        [MODULE_INDEX(DEV_I2CM_API_ID)] = STRUCT_COUNT(DEV_I2CM_FPTRS),
};
