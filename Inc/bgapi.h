/**
  ******************************************************************************
  * @file           bgapi.h
  * @brief          Header for bgapi.c file.
  *                 This file contains the definitions for functions relating
  *                 to BGAPI operations
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BGAPI_H
#define __BGAPI_H

/* Includes */
#include "stm32u5xx_hal.h"
#include "dumo_bglib.h"

/* Defines */


/* Exported types */
/**
  * @brief  BT122 UART Modes structures definition
  */
typedef enum
{
  BGAPI_MODE   = 0x00,
  DATA_MODE    = 0x01
} BT122_UART_MODES;

/* Functions prototypes */
void initializeBGLIB();
void printMACAddress(bd_addr address);
void echoReceived(uint8_t endpoint, unsigned int bytes);
HAL_StatusTypeDef setBT122UARTMode(int mode);

// Test functions
void testBGAPI_Test1();
void testBGAPI_Test2();
void testBGAPI_Test3();

#endif /* __BGAPI_H */
