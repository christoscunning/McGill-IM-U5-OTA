/**
  ******************************************************************************
  * @file           ota.h
  * @brief          Header for ota.c file.
  *                   This file contains the definitions for functions relating
  *                   to the OTA DFU
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OTA_H
#define __OTA_H

/* Includes */
#include "stm32u5xx_hal.h"

/* Defines */

// Address for where to put firmware file in flash
#define FLASH_FIRMWARE_STARTING_ADDR 135266304 // (0x8100000)

/* Structs */
typedef struct __FirmwareInfo {
	HAL_StatusTypeDef status;					/* Status of firmware upload (HAL_OK or HAL_ERROR) */
	uint32_t oldBootloaderVersion;				/* */
	uint16_t major;								/* */
	uint16_t minor;								/* */
	uint16_t patch;								/* */
	uint16_t build;								/* */
	uint16_t newBootloaderVersion;				/* */
	uint16_t hardwareType;						/* */
} FirmwareInfo;

/* Functions prototypes */

// Firmware download functions
int download_firmware(UART_HandleTypeDef *huart, char *firmware, int size);
HAL_StatusTypeDef downloadFirmwareToFlash(UART_HandleTypeDef *huart, uint32_t flashAddress, int size);



// Firmware upload functions
FirmwareInfo uploadFirmwareToBT122(UART_HandleTypeDef *huart, const uint32_t flashAddress, const uint32_t firmwareSize);


// Test functions


#endif /* __OTA_H */
