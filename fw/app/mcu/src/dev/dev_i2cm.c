/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dev_i2cm.c
  * @brief   This file provides code for the configuration
  *          of the I2C master.
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
#include "dev_i2cm_api.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

static I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
HAL_StatusTypeDef I2CM_InitStart(void)
{
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		return HAL_ERROR;
	}
 	 return HAL_OK;
}

HAL_StatusTypeDef I2CM_DeInit(void)
{
	if (HAL_I2C_DeInit(&hi2c1) != HAL_OK)
	{
		return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef I2CM_Write(uint8_t address, uint8_t *data, uint16_t size)
{
	HAL_StatusTypeDef ret = HAL_OK;

	if (HAL_I2C_Master_Transmit(&hi2c1, (address << 1), data, size, I2C_TIMEOUT) != HAL_OK) {
		if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF) {
			return HAL_ERROR;
		}
	}
	return ret; 
}

HAL_StatusTypeDef I2CM_Read(uint8_t address, uint8_t *data, uint16_t size)
{
	HAL_StatusTypeDef ret = HAL_OK;

	if (HAL_I2C_Master_Receive(&hi2c1, (address << 1), data, size, I2C_TIMEOUT) != HAL_OK) {
		if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF) {
			return HAL_ERROR;
		}
	}
	return ret; 
}

HAL_StatusTypeDef I2CM_Read_Sr(uint8_t address, uint8_t reg, uint8_t *data, uint16_t size)
{
	HAL_StatusTypeDef ret = HAL_OK;

	if (HAL_I2C_Mem_Read(&hi2c1, (address << 1), reg, I2C_MEMADD_SIZE_8BIT, data, size, I2C_TIMEOUT) != HAL_OK) {
		if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF) {
			return HAL_ERROR;
		}
		ret = HAL_ERROR;
	}
	return ret; 
}

HAL_StatusTypeDef I2CM_IsDeviceReady(uint8_t address, uint32_t trials)
{
	HAL_StatusTypeDef ret = HAL_OK;
	
	if (HAL_I2C_IsDeviceReady(&hi2c1, (address << 1), trials, I2C_TIMEOUT) != HAL_OK) {
		if (HAL_I2C_GetError(&hi2c1) != HAL_I2C_ERROR_AF) {
			return HAL_ERROR;
		}
		ret = HAL_ERROR;
	}
	return ret; 
}
