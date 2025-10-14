api = {
    'I2C_TIMEOUT': {
        'type': 'define',
        'value': '(5000)',
        'comment': 'Timeout de I2C en milisegundos.',
    },
    'I2CM_InitStart': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Inicializa I2C1 a 400 kHz, 7-bit, sin dual address.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    },
    'I2CM_DeInit': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Desinicializa I2C1.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    },
    'I2CM_Write': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Escribe un buffer en un esclavo I²C.',
        'params': {
            'address': {
                'type': 'uint8_t',
                'dir': 'in',
                'comment': 'Dirección 7-bit del esclavo (p.ej. 0x68).',
            },
            'data': {
                'type': 'uint8_t[512]',
                'dir': 'in',
                'len': 'size',
                'comment': 'Puntero al buffer a transmitir.',
            },
            'size': {
                'type': 'uint16_t',
                'dir': 'in',
                'comment': 'Cantidad de bytes a transmitir.',
            },
        },
    },
    'I2CM_Read': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Lee un buffer crudo desde un esclavo I²C (no lee dirección interna).',
        'params': {
            'address': {
                'type': 'uint8_t',
                'dir': 'in',
                'comment': 'Dirección 7-bit del esclavo (p.ej. 0x68).',
            },
            'data': {
                'type': 'uint8_t[512]',
                'dir': 'out',
                'len': 'size',
                'comment': 'Puntero al buffer de recepción.',
            },
            'size': {
                'type': 'uint16_t',
                'dir': 'in',
                'comment': 'Cantidad de bytes a recibir.',
            },
        },
    },
    'I2CM_Read_Sr': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Lee bytes desde un registro interno (sub-address read).',
        'params': {
            'address': {
                'type': 'uint8_t',
                'dir': 'in',
                'comment': 'Dirección 7-bit del esclavo (p.ej. 0x68).',
            },
            'reg': {
                'type': 'uint8_t',
                'dir': 'in',
                'comment': 'Dirección interna (8-bit) de inicio.',
            },
            'data': {
                'type': 'uint8_t[512]',
                'dir': 'out',
                'len': 'size',
                'comment': 'Buffer de salida.',
            },
            'size': {
                'type': 'uint16_t',
                'dir': 'in',
                'comment': 'Número de bytes a leer.',
            },
        },
    },
    'I2CM_IsDeviceReady': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Verifica si un esclavo responde (ACK) en la dirección dada.',
        'params': {
            'address': {
                'type': 'uint8_t',
                'dir': 'in',
                'comment': 'Dirección 7-bit.',
            },
            'trials': {
                'type': 'uint32_t',
                'dir': 'in',
                'comment': 'Número de reintentos.',
            },
        },
    },
}