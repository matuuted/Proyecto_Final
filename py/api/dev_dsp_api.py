
from api_dbg_hdr import *
from .caller import *

# API Functions


'''

#
#   \return u8
    \param [in] port (u8) DSP port number.
    \param [in] addr (u32) Register address.
    \param [in] value (u32) Value to write.
Writes a value to a DSP register.
    RTOS Info: Any
'''
def DSP_RegWritre(port, addr, value):
    argType = 0x01
    args = [
            arg('port','u8', port),
            arg('addr','u32', addr),
            arg('value','u32', value),
    ]
    retVal = call(DBG_DEV_DSP_API_ID, DBG_DEV_DSP_API["DSP_REGWRITRE"], argType, args)

    return (retVal)
