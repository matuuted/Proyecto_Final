api = {
    'I2CS_SLAVE_ADDR': {
        'type': 'define',
        'value': '0x50',
        'comment': 'I2C slave address.',
    },
    'I2CS_STATE': {
        'type': 'enum',
        'comment': 'I2C slave address.',
        'nums': {
            'READY': {
                'value': 0,
                'comment': 'I2C slave is ready.',
            },
            'GET_ADDR': {
                'value': 1,
                'comment': 'I2C slave is getting address.',
            },
            'SET_ADDR': {
                'value': 2,
                'comment': 'I2C slave is setting address.',
            },
            'GET_DATA': {
                'value': 3,
                'comment': 'I2C slave is getting data.',
            },
            'SET_DATA': {
                'value': 4,
                'comment': 'I2C slave is sending data.',
            },
            'NACK': {
                'value': 5,
                'comment': 'I2C slave is not acknowledging.',
            },
        },
    },
    'I2CS_InitStart': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Initializes the I2C slave module.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    },
    'I2CS_DeInit': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Deinitializes the I2C slave module.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    },
    'I2C2_EnableAcknowledge': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Enables the I2C slave module.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    },
    'I2C2_DisableAcknowledge': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'Any',
        'comment': 'Disables the I2C slave module.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    }
}
