/**
  ******************************************************************************
  * @file           flash.h
  * @brief          Header for flash.c file.
  *                 This file contains the definitions for functions relating
  *                 to the flash memory on u5a5 device.
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H
#define __FLASH_H

/* Includes */
#include "stm32u5xx_hal.h"

/* Defines */


/* Constants */


/* Functions prototypes */

// Error handling
void printFlashError(uint32_t flashErrorCode);
void clearAllFlashFlags();

// Flash Options Bytes Queries
int areFlashBanksSwapped();

// Read functions
uint32_t readFlash(int baseAddress, int offset);
HAL_StatusTypeDef readFlashPage(int page, char *buffer);
void printFlashData(const uint32_t FLASH_START_ADDR, const uint32_t FLASH_END_ADDR);

// Write functions
HAL_StatusTypeDef writeFlash(uint32_t flashAddress, uint32_t dataAddress);
HAL_StatusTypeDef writeFlashPage(int page, const char *buffer);
HAL_StatusTypeDef writeFlashLarge(uint32_t flashAddress, const char * buffer, const int length);

// Erase functions
HAL_StatusTypeDef eraseFlashPage(int page);

// Test functions
void flashTest_1();
void flashTest_2();
void flashTest_3();
void flashTest_4();
void flashTest_5();
HAL_StatusTypeDef compareBuffers(char *buffer1, char *buffer2, int bufferLength);


#endif /* __FLASH_H */
