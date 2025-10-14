/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dev_i2cs.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "api.h"
#include "dev_i2cs_api.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx.h"
#include "sys_dbg_api.h"


static I2C_HandleTypeDef hi2c2;

volatile uint8_t state = I2CS_STATE_READY;
static uint8_t addr = 0;
static uint16_t count = 0;


static void I2C2_EnableAck(I2C_TypeDef *I2Cx)
{
  SET_BIT(I2Cx->CR1, I2C_CR1_ACK);
}

/* I2C2 init function */
HAL_StatusTypeDef I2C2_EnableAcknowledge(void)
{
  if (!LL_I2C_IsEnabled(I2C2)){
    LL_I2C_Enable(I2C2);
  }else {
    return HAL_ERROR;
  }
  return HAL_OK;
}

HAL_StatusTypeDef I2C2_DisableAcknowledge(void)
{
  if (LL_I2C_IsEnabled(I2C2)){
	LL_I2C_Disable(I2C2);
  }else {
	return HAL_ERROR;
  }
  return HAL_OK;
}


HAL_StatusTypeDef I2CS_InitStart(void)
{
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = (I2CS_SLAVE_ADDR << 1);
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
    return HAL_ERROR;
  }

  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
	return HAL_ERROR;
  }

  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK) {
	return HAL_ERROR;
  }

  /* Enable the Interrupt sources */
  LL_I2C_EnableIT_EVT(hi2c2.Instance);
  LL_I2C_EnableIT_ERR(hi2c2.Instance);
  LL_I2C_EnableIT_BUF(hi2c2.Instance);
  I2C2_EnableAck(hi2c2.Instance);

  addr = 0;

  return HAL_OK;
}

HAL_StatusTypeDef I2CS_DeInit(void)
{
	if (HAL_I2C_DeInit(&hi2c2) != HAL_OK)
	{
		return HAL_ERROR;
	}
	return HAL_OK;
}


void I2C2_EV_IRQHandler(void)
{
	// Dirección recibida
	if(LL_I2C_IsActiveFlag_ADDR(I2C2))
	{
		uint8_t direction = (LL_I2C_GetTransferDirection(I2C2));

		LL_I2C_ClearFlag_ADDR(I2C2);

		count = 0;

		// Master escribe al slave
		if (direction == 0x00) {
			addr = 0;
			state = I2CS_STATE_GET_ADDR;
		} else { // Master lee del slave
			if (state == I2CS_STATE_READY){ // Read Puro. 
				addr = 0; // Si no se ha escrito nada antes, leo desde el principio
			} 
			state = I2CS_STATE_SET_DATA;
		}
	}
	// Recepción de datos
	if (LL_I2C_IsActiveFlag_RXNE(I2C2))
	{
		if (state == I2CS_STATE_GET_ADDR) {
			addr = I2C2->DR;  // La primera posición del buffer es la dirección de escritura

			// command.buffer[addr++] = I2C2->DR;
			state = I2CS_STATE_SET_ADDR;
		} else if (state == I2CS_STATE_SET_ADDR) {
			if (count < COMMAND_SIZE) {
				command.buffer[count ++] = addr; // La primera posición del buffer es la dirección de escritura
				command.buffer[count++] = I2C2->DR;
			} else {
				(void)I2C2->DR;
			}
			state = I2CS_STATE_GET_DATA;
		}
		else { // I2CS_STATE_GET_DATA
			if (count < COMMAND_SIZE){
				command.buffer[count++] = I2C2->DR;
			} else {
				(void)I2C2->DR;
			}
		}
	}

	// Transmisión de datos
	if (LL_I2C_IsActiveFlag_TXE(I2C2)){
		if (state == I2CS_STATE_SET_DATA) {
			if (addr + count < COMMAND_SIZE) {
				I2C2->DR = command.buffer[addr + count];
				count++;
			} else {
				I2C2->DR = 0x00; // Si se pide más allá del buffer, se envía 0
			}
		} 
	}

	// STOP detectado
	if (LL_I2C_IsActiveFlag_STOP(I2C2))
	{
		if (state == I2CS_STATE_SET_ADDR) {
            // Caso atípico: sólo 1 byte y STOP → lo consideramos escritura de 1 byte
            if (count < COMMAND_SIZE) {
                command.buffer[count++] = addr;
            }
        }


		// Si se estaba recibiendo data, la transacción termina acá y se notifica
		if (count > 0) {
			osThreadFlagsSet(DebugTaskHandle, OS_NOTIFY_I2C_COMMAND);
		}

		LL_I2C_ClearFlag_STOP(I2C2);
		state = I2CS_STATE_READY;

		// if (state == I2CS_STATE_GET_DATA){
		// 	LL_I2C_ClearFlag_STOP(I2C2);
		// 	state = I2CS_STATE_READY;

		// 	osThreadFlagsSet(DebugTaskHandle, OS_NOTIFY_I2C_COMMAND);

		// }
	}

	// NACK detectado
	// if (LL_I2C_IsActiveFlag_AF(I2C2))
	// {
	// 	state = I2CS_STATE_READY;
	// 	LL_I2C_ClearFlag_AF(I2C2);
	// }
}

void I2C2_ER_IRQHandler(void)
{
	if (LL_I2C_IsActiveFlag_AF(I2C2))
	{
		LL_I2C_ClearFlag_AF(I2C2);
		state = I2CS_STATE_READY;
		return; // NACK final
	}
	if (LL_I2C_IsActiveFlag_BERR(I2C2))
	{
		LL_I2C_ClearFlag_BERR(I2C2);
	}
	if (LL_I2C_IsActiveFlag_ARLO(I2C2))
	{
		LL_I2C_ClearFlag_ARLO(I2C2);
	}
	if (LL_I2C_IsActiveFlag_OVR(I2C2))
	{
		LL_I2C_ClearFlag_OVR(I2C2);
	}
}