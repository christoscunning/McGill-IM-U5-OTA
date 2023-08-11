/**
  ******************************************************************************
  * @file           util.h
  * @brief          Header for util.c file.
  *                   This file contains the definitions for functions providing
  *                   general utility.
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UTIL_H
#define __UTIL_H

/* Includes */
#include "stm32u5xx_hal.h"

/* Defines */

/* Constants */


/* Functions prototypes */

// General Utility
void printBuffer(char *buffer, int length, char *format);

// Endianness
int isBigEndian();
HAL_StatusTypeDef fixEndianness(uint8_t *buffer, int length);

// Hashing
HAL_StatusTypeDef computeHashFromFlash(HASH_HandleTypeDef *hhash, uint32_t flashAddress, int size, char *digest);


#endif /* __UTIL_H */
