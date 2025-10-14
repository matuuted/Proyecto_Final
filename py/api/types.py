import struct
from decimal import Decimal

DIE = 0
GO_REG = 0x0
IDLE_VAL = 0x0
GO_VAL = 0x2
PARAM_REG = 0x4

TLV_TYPE = 0
TLV_LENGTH = 1
TLV_VALUE = 4
TLV_HDRLEN = 4

API_ERR_COMMS   = -20
API_ERR_OS      = -4
API_ERR_TIMEOUT = -3

def u16_to_f16(value):
    (sign, digits, exponent) =  Decimal(value).as_tuple()
    exp = len(digits) + exponent - 1
    man = int(Decimal(100.0) * Decimal(value).scaleb(-exp).normalize())
    return man | ((exp + 22) << 11)


def f16_to_u16(value):
    # Convert ul6 to f16
    exp = ((value >> 11) - 22)
    man = (value & 0x7ff) / 100.0
    return man * pow(10, exp)

class baseType:
    def __init__(self, name, ul, idx, fmt, to_fn=None, from_fn=None):
        self.name = name
        self.length = ul
        self.idx = idx
        self.fmt = fmt
        self.to_fn = to_fn
        self.from_fn = from_fn

    def to_data(self, value):
        if self.to_fn is not None:
            value = self.to_fn(value)
        return [ord(c) if type(c) is str else c for c in struct.pack(self.fmt, value)]

    def from_data(self, data):
        value = struct.unpack(self.fmt, bytes(bytearray(data)))[0]
        if self.from_fn is not None:
            value = self.from_fn(value)
        return value

ARG_IS_POINTER = 0x80

OS_ERR = {}
OS_ERR = {
    'API_ERR_TIMEOUT'       : {'err_val': -3},
    'API_ERR_UNSUPPORTED'   : {'err_val': -2},
    'API_ERR_INVALID'       : {'err_val': -1},
    'API_OK'                : {'err_val': 0},
    'API_BUSY'              : {'err_val': 1},
    'API_GO'                : {'err_val': 2},
    'API_ABORT'             : {'err_val': 4},
    }


# BASE_TYPES = {
#     'u8':  baseType('u8',  1, 0, 'B'),
#     's8':  baseType('s8',  1, 1, 'b'),
#     'u16': baseType('u16', 2, 2, '<H'),
#     's16': baseType('s16', 2, 3, '<h'),
#     'u32': baseType('u32', 4, 4, '<I'),
#     's32': baseType('s32', 4, 5, '<i'),
#     'u64': baseType('u64', 8, 6, '<Q'),
#     's64': baseType('s64', 8, 7, '<q'),
#     'f16': baseType('f16', 2, 7, '<H', u16_to_f16, f16_to_u16)
# }

BASE_TYPES = {
    'uint8_t':   baseType('uint8_t',   1,  0, 'B'),
    'int8_t':    baseType('int8_t',    1,  1, 'b'),
    'uint16_t':  baseType('uint16_t',  2,  2, '<H'),
    'int16_t':   baseType('int16_t',   2,  3, '<h'),
    'uint32_t':  baseType('uint32_t',  4,  4, '<I'),
    'int32_t':   baseType('int32_t',   4,  5, '<i'),
    'uint64_t':  baseType('uint64_t',  8,  6, '<Q'),
    'int64_t':   baseType('int64_t',   8,  7, '<q'),
    'float16_t': baseType('float16_t', 2,  8, '<H', u16_to_f16, f16_to_u16)
}