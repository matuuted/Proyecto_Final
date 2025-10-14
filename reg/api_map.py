import files
from files import dev_i2cs, dev_i2cm, sys_dbg


group_size = 0x40
package_name = "apigen"

api = {
    'dev': {
        'I2CS': dev_i2cs.api,
        'I2CM': dev_i2cm.api,
    },
    'sys': {
        'DBG': sys_dbg.api,
    },
}

output_dirs = {
    'h_api_gen': '../fw/app/mcu/inc/api',
    'c_api_gen': '../fw/app/mcu/src/api',
    'h_api_modules_gen': '../fw/app/mcu/inc/api',
    'c_api_modules_gen': '../fw/app/mcu/src/api',
    'py_api_gen': '../py/api',
    'py_api_dbg_hdr_gen': '../py',
    'py_api_init_gen': '../py/api',
    'c_gen': 'out',
}