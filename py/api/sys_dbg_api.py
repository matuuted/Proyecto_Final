
from api_dbg_hdr import *
from .caller import *

# API Functions


'''

#
#   \return HAL_StatusTypeDef
Initializes the RTOS kernel and related components.
    RTOS Info: NONE
'''
def DBG_InitOS():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_SYS_DBG_API_ID, DBG_SYS_DBG_API["DBG_INITOS"], argType, args)

    return (retVal)
        

'''

#
#   \return void
Performs the iteration of the debug task.
    RTOS Info: NONE
'''
def DBG_Iter():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_SYS_DBG_API_ID, DBG_SYS_DBG_API["DBG_ITER"], argType, args)

    return (retVal)
        
