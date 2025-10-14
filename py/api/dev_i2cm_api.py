
from api_dbg_hdr import *
from .caller import *

# API Functions

# Timeout de I2C en milisegundos.
DEV_I2CM_API_I2C_TIMEOUT = (5000)


'''

#
#   \return HAL_StatusTypeDef
Inicializa I2C1 a 400 kHz, 7-bit, sin dual address.
    RTOS Info: Any
'''
def I2CM_InitStart():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_DEV_I2CM_API_ID, DBG_DEV_I2CM_API["I2CM_INITSTART"], argType, args)

    return (retVal)
        

'''

#
#   \return HAL_StatusTypeDef
Desinicializa I2C1.
    RTOS Info: Any
'''
def I2CM_DeInit():
    argType = 0x00
    args = [
    ]
    retVal = call(DBG_DEV_I2CM_API_ID, DBG_DEV_I2CM_API["I2CM_DEINIT"], argType, args)

    return (retVal)
        

'''

#
#   \return HAL_StatusTypeDef
    \param [in] address (uint8_t) Dirección 7-bit del esclavo (p.ej. 0x68).
    \param [in] data (uint8_t[512]) Puntero al buffer a transmitir.
    \param [in] size (uint16_t) Cantidad de bytes a transmitir.
Escribe un buffer en un esclavo I²C.
    RTOS Info: Any
'''
def I2CM_Write(address, data):
    argType = 0x07
    args = [
            arg('address','uint8_t', address),
            arg('data','uint8_t[{}]'.format(len(data)), data),
            arg('size','uint16_t', len(data)),
    ]
    retVal = call(DBG_DEV_I2CM_API_ID, DBG_DEV_I2CM_API["I2CM_WRITE"], argType, args)

    return (retVal)
        

'''

#
#   \return HAL_StatusTypeDef
    \param [in] address (uint8_t) Dirección 7-bit del esclavo (p.ej. 0x68).
    \param [out] data (uint8_t[512]) Puntero al buffer de recepción.
    \param [in] size (uint16_t) Cantidad de bytes a recibir.
Lee un buffer crudo desde un esclavo I²C (no lee dirección interna).
    RTOS Info: Any
'''
def I2CM_Read(address, size):
    argType = 0x07
    args = [
            arg('address','uint8_t', address),
            arg('data','uint8_t[{}]'.format(size), arg()),
            arg('size','uint16_t', size),
    ]
    retVal = call(DBG_DEV_I2CM_API_ID, DBG_DEV_I2CM_API["I2CM_READ"], argType, args)
    data = args[1].value

    return (retVal, data)
        

'''

#
#   \return HAL_StatusTypeDef
    \param [in] address (uint8_t) Dirección 7-bit del esclavo (p.ej. 0x68).
    \param [in] reg (uint8_t) Dirección interna (8-bit) de inicio.
    \param [out] data (uint8_t[512]) Buffer de salida.
    \param [in] size (uint16_t) Número de bytes a leer.
Lee bytes desde un registro interno (sub-address read).
    RTOS Info: Any
'''
def I2CM_Read_Sr(address, reg, size):
    argType = 0x0F
    args = [
            arg('address','uint8_t', address),
            arg('reg','uint8_t', reg),
            arg('data','uint8_t[{}]'.format(size), arg()),
            arg('size','uint16_t', size),
    ]
    retVal = call(DBG_DEV_I2CM_API_ID, DBG_DEV_I2CM_API["I2CM_READ_SR"], argType, args)
    data = args[2].value

    return (retVal, data)
        

'''

#
#   \return HAL_StatusTypeDef
    \param [in] address (uint8_t) Dirección 7-bit.
    \param [in] trials (uint32_t) Número de reintentos.
Verifica si un esclavo responde (ACK) en la dirección dada.
    RTOS Info: Any
'''
def I2CM_IsDeviceReady(address, trials):
    argType = 0x01
    args = [
            arg('address','uint8_t', address),
            arg('trials','uint32_t', trials),
    ]
    retVal = call(DBG_DEV_I2CM_API_ID, DBG_DEV_I2CM_API["I2CM_ISDEVICEREADY"], argType, args)

    return (retVal)
        
