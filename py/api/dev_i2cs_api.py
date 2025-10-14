
from api_dbg_hdr import *
from .caller import *

# API Functions

# I2C slave address.
DEV_I2CS_API_I2CS_SLAVE_ADDR = 0x50

# I2C slave address.
I2CS_STATE = {}
# I2C slave is ready.
I2CS_STATE['READY'] = 0
# I2C slave is getting address.
I2CS_STATE['GET_ADDR'] = 1
# I2C slave is setting address.
I2CS_STATE['SET_ADDR'] = 2
# I2C slave is getting data.
I2CS_STATE['GET_DATA'] = 3
# I2C slave is sending data.
I2CS_STATE['SET_DATA'] = 4
# I2C slave is not acknowledging.
I2CS_STATE['NACK'] = 5


'''

#
#   \return HAL_StatusTypeDef
Initializes the I2C slave module.
    RTOS Info: Any
'''
def I2CS_InitStart():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_DEV_I2CS_API_ID, DBG_DEV_I2CS_API["I2CS_INITSTART"], argType, args)

    return (retVal)
        

'''

#
#   \return HAL_StatusTypeDef
Deinitializes the I2C slave module.
    RTOS Info: Any
'''
def I2CS_DeInit():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_DEV_I2CS_API_ID, DBG_DEV_I2CS_API["I2CS_DEINIT"], argType, args)

    return (retVal)
        

'''

#
#   \return HAL_StatusTypeDef
Enables the I2C slave module.
    RTOS Info: Any
'''
def I2C2_EnableAcknowledge():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_DEV_I2CS_API_ID, DBG_DEV_I2CS_API["I2C2_ENABLEACKNOWLEDGE"], argType, args)

    return (retVal)
        

'''

#
#   \return HAL_StatusTypeDef
Disables the I2C slave module.
    RTOS Info: Any
'''
def I2C2_DisableAcknowledge():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_DEV_I2CS_API_ID, DBG_DEV_I2CS_API["I2C2_DISABLEACKNOWLEDGE"], argType, args)

    return (retVal)
        
