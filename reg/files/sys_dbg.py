api = {
    'DBG_InitOS': {
        'type': 'function',
        'returns': 'HAL_StatusTypeDef',
        'rtos': 'NONE',
        'comment': 'Initializes the RTOS kernel and related components.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    },
    'DBG_Iter': {
        'type': 'function',
        'returns': 'void',
        'rtos': 'NONE',
        'comment': 'Performs the iteration of the debug task.',
        'params': {
            '_void': {
                'comment': '',
            },
        },
    }
}
