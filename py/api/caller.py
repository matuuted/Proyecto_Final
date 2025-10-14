import time
import re
import os
from . import types
from Drivers.sub20_py.sub20_port import Sub20

dev = Sub20()
dev.i2c_set_freq(100_000)

I2C_ADDRESS = 0x50
SA      = I2C_ADDRESS  
ADDR_SZ = 1     


class arg:
    def __init__(self, name="arg", argtype_name="uint32_t", value=None):
        self.name = name
        self.argtype_name = argtype_name

        if isinstance(value, arg):
            self.value_is_arg = True
        else:
            self.value_is_arg = False

        if argtype_name == "uint32_t":
            self.is_tlv = False
        else:
            self.is_tlv = True

        r = re.match(r'(\S+)\[(\d+)\]', self.argtype_name)
        if r:
            self.count = int(r.group(2))
            self.base_type_name = r.group(1)
            self.value = [0] * self.count
            self.pointer = True
        else:
            self.count = 1
            self.value = 0
            self.base_type_name = self.argtype_name
            self.pointer = False

        self.base_type = types.BASE_TYPES[self.base_type_name]
        if self.pointer:
            self.arg_type = self.base_type.idx | types.ARG_IS_POINTER
        else:
            self.arg_type = self.base_type.idx

        self.length = self.count * self.base_type.length
        if self.is_tlv:
            self.length += (types.TLV_HDRLEN + ((4 - (self.length % 4)) & 0x3))

        if self.value is not None:
            self.set_value(value)

    def set_value(self, value):
        if self.value_is_arg:
            value.value = self.value
            self.value_arg = value
        else:
            self.value_arg = None
            self.value = value

    def put(self):
        if self.is_tlv:
            data = [0] * types.TLV_HDRLEN
            data[types.TLV_TYPE] = self.arg_type
            data[types.TLV_LENGTH:types.TLV_LENGTH+2] = types.BASE_TYPES['uint16_t'].to_data(self.length)
        else:
            data = []

        if self.pointer:
            for value in (self.value + [0] * (self.count - len(self.value)))[0:self.count]:
                data.extend(self.base_type.to_data(value))
        else:
            data.extend(self.base_type.to_data(self.value))

        if len(data) != self.length:
            data.extend([0] * ((4 - (len(data) % 4)) & 0x3))
        return data

    def get(self, data):
        if self.is_tlv:
            idx = types.TLV_VALUE
            self.value = []
            for _ in range(self.count):
                self.value.append(
                    self.base_type.from_data(data[idx:idx + self.base_type.length])
                )
                idx += self.base_type.length
        else:
            self.value = self.base_type.from_data(data)

        if self.value_arg is not None:
            if self.pointer and self.count == 1:
                self.value_arg.value = self.value[0]
            else:
                self.value_arg.value = self.value

def __getretValname(err_id):
    error_name = 'NULL'
    for name in types.OS_ERR:
        if types.OS_ERR[name]['err_val'] == err_id:
            error_name = name
            break
    return error_name

def log_call(retVal, module_id, function_id):
    error_name = __getretValname(retVal)
    log_dir = os.path.join(os.path.dirname(__file__), "..", "reg", "log")

    os.makedirs(log_dir, exist_ok=True)

    log_path = os.path.join(log_dir, "call.log")
    error_name = __getretValname(retVal)
    with open(log_path, "a") as f:
        print("Call Error: %d (%s) in module %d function %d" % (retVal, error_name, module_id, function_id))
    return

def call(module_id, function_id, argTypes, args):
    data = []

    # Procesa los argumentos
    for a in args:
        # si es u32, asegura que el offset sea múltiplo de 4 bytes para el firmware
        if a.argtype_name == 'uint32_t' and (len(data) % 4):
            data.extend([0] * (4 - (len(data) % 4)))
        data.extend(a.put())

    # Agrega parámetros para la llamada a DBG_Call


    hdr = [module_id, function_id, argTypes] + data
    dev.i2c_writell(SA, bytes(x & 0xFF for x in hdr), ma=types.GO_VAL, ma_sz=ADDR_SZ)

    timeout = 5
    start_time = time.time()

    while (1):
        try:
            raw = dev.i2c_readll(SA, 1, ma=types.GO_REG, ma_sz=ADDR_SZ)
            status, retVal = 0, raw[0]
        except Exception:
            status, retVal = -1, 0

        retVal &= 0xff
        if (status != 0):
            retVal = types.API_ERR_COMMS
        else:
            if (retVal == 0): break  # HAL_OK

        if (retVal < 0):
            log_call(retVal, module_id, function_id)
            break
        time.sleep(0.05)

        if (timeout != 0 and time.time() - start_time > timeout):
            return types.API_ERR_TIMEOUT

    length = 0
    for a in args: length += a.length

    if length > 0:
        # (status, data) = mod_reg.caller.mod_reg_get_block(types.DIE, mod_reg.caller.gen_addr(types.PARAM_REG), length)
        try:
            raw = dev.i2c_readll(SA, length, ma=types.PARAM_REG, ma_sz=ADDR_SZ)
            status, data = 0, list(raw)
        except Exception:
            status, data = -1, []



        data[0] &= 0xff
        if status != 0:
            retVal = types.API_ERR_COMMS
            log_call(retVal, module_id, function_id)
        else:
            offset = 0
            for a in args:
                a.get(data[offset:offset + a.length])
                offset += a.length
    return retVal