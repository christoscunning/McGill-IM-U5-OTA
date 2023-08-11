/**
 * @file uart.h
 * @author Silicon Labs R&D Team
 * @date October 2020
 *
 ********************************************************************************************
 * <b> (C) Copyright 2020 Silicon Labs, http://www.silabs.com </b>
 ********************************************************************************************
 * This file is licensed under the Silicon Labs License Agreement. For details see the file:
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 * Before using this software for any purpose, you must agree to the terms of that agreement.
 *
 ********************************************************************************************
 *
 * @brief DESC.
 */

#ifndef UART_H
#define UART_H

/* Includes */
#include "stm32u5xx_hal.h"

/* Defines */
//#define UART_DEBUG 0

#define MAX_NUMBER_UART_HANDLES 10
#define UART_IT_BUFFER_LENGTH 12288 // 12 KB

/* Constants */


/* Function prototypes */

// SETUP
int register_UART(int huartNum, UART_HandleTypeDef *huart);
int get_UART_num(UART_HandleTypeDef *huart);
void checkConnection(UART_HandleTypeDef *huart);

// BLOCKING
int uart_rx(UART_HandleTypeDef *huart, int data_length, char *data);
int uart_tx(UART_HandleTypeDef *huart, int data_length, const char *data);

// INTERRUPT
int uart_rx_it(UART_HandleTypeDef *huart, int data_length, char *data);
int uart_rx_it_put(int huartNum, int data_length, char *data);
int uart_rx_it_get(int huartNum, int data_length, char *data);
int uart_rx_it_get_length(int huartNum);
void uart_rx_it_clear_buffer(int huartNum);

// TESTING
void testUARTInterruptBuffer_Test1(int huartNum);
void testUARTInterruptBuffer_Test2(UART_HandleTypeDef *huart);

/*******************************************************************************/

#endif /* UART_H */


