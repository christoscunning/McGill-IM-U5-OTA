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
#ifdef DEBUG
#ifndef UART_DEBUG
//#define UART_DEBUG 0
#endif /* UART_DEBUG */
#endif /* DEBUG */

#define MAX_NUMBER_UART_HANDLES 10
#define UART_IT_BUFFER_LENGTH 12288 // 12 KB

/* Constants */


/* Function prototypes */

/**
 * @brief   Associates the huartNum with a UART_HandleTypeDef pointer.
 * @param   huartNum The UART number to be associated with the UART handle pointer.
 * @param   huart The pointer to the UART handle.
 * @retval  Returns -1 if there is an error, otherwise, returns huartNum if successful.
 */
int register_UART(int huartNum, UART_HandleTypeDef *huart);

/**
 * Returns the huartNum associated with the specified UART_HandleTypeDef pointer.
 * @param   huart
 * @retval  The associated huartNum
 */
int get_UART_num(UART_HandleTypeDef *huart);

// BLOCKING

/**
 * @brief   Read data from serial port. The function will block until the desired amount has been read or an error occurs.
 * @param   data_length The amount of bytes to read.
 * @param   data Buffer used for storing the data.
 * @retval  The amount of bytes read or -1 on failure.
 */
int uart_rx(UART_HandleTypeDef *huart, int data_length, char *data);

/**
 * @brief   Write data to serial port. The function will block until the desired amount has been written or an error occurs.
 * @param   data_length The amount of bytes to write.
 * @param   data Data to write.
 * @retval  The amount of bytes written or -1 on failure.
 */
int uart_tx(UART_HandleTypeDef *huart, int data_length, const char *data);


// INTERRUPT

/**
 * @brief   Same as uart_rx, except this function utilizes the UART interrupt buffer.
 */
int uart_rx_it(UART_HandleTypeDef *huart, int data_length, char *data);

/**
 * @brief   When data is received through UART interrupt, call this function to store it in a buffer until
 * 			it is read back using uart_rx_it_get().
 */
int uart_rx_it_put(int huartNum, int data_length, char *data);

/**
 * @brief   Get data_lengths numbers of bytes from UART RX buffer.
 */
int uart_rx_it_get(int huartNum, int data_length, char *data);

/**
 * @brief   Returns the current length of the UART RX buffer.
 */
int uart_rx_it_get_length(int huartNum);

/**
 * @brief   Clears the rx interrupt buffer for the specified UART handle.
 */
void uart_rx_it_clear_buffer(int huartNum);



// TESTING
void testUARTInterruptBuffer_Test1(int huartNum);
void testUARTInterruptBuffer_Test2(UART_HandleTypeDef *huart);

/*******************************************************************************/

#endif /* UART_H */


