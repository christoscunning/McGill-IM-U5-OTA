/**
 * @file uart.c
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
 *	Modified from original version.
 *
 * @brief Functions to simplify interaction with UART.
 */
#include <stdint.h>
#include <stdio.h>
#include "uart.h"

/*
 * Array that tracks associated between huart handle pointer, and uart number.
 * ie: &huart2 <=> 2
 */
UART_HandleTypeDef *UARTHandles[MAX_NUMBER_UART_HANDLES] = {NULL};

/**
 * Associates the huartNum with a UART_HandleTypeDef pointer.
 * This is mainly so that uart_rx and uart_rx_it can have the same function prototype.
 */
int register_UART(int huartNum, UART_HandleTypeDef *huart) {
	if (huartNum >= MAX_NUMBER_UART_HANDLES || huartNum < 1) {
		return -1;
	}
	UARTHandles[huartNum] = huart;
	return huartNum;
}

/**
 * Checks if huart pointer is in the UARTHandles array, if so returns the UART
 * number else returns -1.
 */
int get_UART_num(UART_HandleTypeDef *huart){
	// get huartNum
	int huartNum = -1;
	for (int i = 0; i < MAX_NUMBER_UART_HANDLES; i++) {
		if (UARTHandles[i] == huart) {
			huartNum = i;
		}
	}
	return huartNum;
}

/*******************************************************************************/
/*							Polling UART									   */
/*******************************************************************************/

/**
 * @brief   Receives a <dataLength> number of bytes through the specified UART
 * 			interface in polling mode.
 *
 * @param   huart The handle of the UART that will be used to receive the data.
 * @param   dataLength The number of bytes that will be received. This function
 * 			will block until <dataLength> number of bytes are received.
 * @param   data Pointer to the char array in which the received data will be written.
 * @retval  The number of bytes received and placed in the <data> char buffer.
 */
int uart_rx(UART_HandleTypeDef *huart, int dataLength, char *data) {
	/* Variable for storing function return values. */
	HAL_StatusTypeDef ret;
	/* The amount of bytes still needed to be read. */
	uint16_t data_to_read = dataLength;
	/* The amount of bytes read. */
	uint16_t data_read;

#ifdef UART_DEBUG
  printf("uart_rx() - dataLength: %d\r\n", dataLength);
#endif

	while (data_to_read) {
		ret = HAL_UART_Receive(huart, (uint8_t*) data, 1, HAL_MAX_DELAY);
		data_read = 1;
		if (ret != HAL_OK) {
			if (ret == HAL_BUSY) {
				continue;
			}
			return -1;
		} else {
			if (!data_read) {
				continue;
			}
		}
		data_to_read -= data_read;
		data += data_read;
	}

#ifdef UART_DEBUG
  for (ret = 0; ret < dataLength; ++ret) {
    printf("%02X", (data - dataLength)[ret]);
  }

  printf("\r\n");
#endif

	return dataLength;
}

/**
 * @brief   Transmit a specified number of bytes using the specified UART in blocking mode.
 *
 * @param   huart The handle of the UART used to transmit the data.
 * @param   dataLength The number of bytes that will be transmitted.
 * @param   data Pointer to the char array containing the data to be transmitted. Should be
 * 			at least <dataLength> bytes long, or else undefined behavior will result.
 * @retval  The number of bytes transmitted.
 */
int uart_tx(UART_HandleTypeDef *huart, int dataLength, const char *data) {
	/* Variable for storing function return values. */
	HAL_StatusTypeDef ret;
	/* The amount of bytes written. */
	uint16_t data_written;
	/* The amount of bytes still needed to be written. */
	uint16_t data_to_write = dataLength;

#ifdef UART_DEBUG
  printf("uart_tx() - dataLength: %d\r\n", dataLength);

  for (int i = 0; i < dataLength; ++i) {
	  unsigned char c = data[i];
    printf("%02X", c);
  }

  printf("\r\n");
#endif

	while (data_to_write) {
		do {
			ret = HAL_UART_Transmit(huart, (uint8_t*) data, 1, HAL_MAX_DELAY);
		} while (ret != HAL_OK);
		data_written = 1;
		if (ret != HAL_OK) {
			printf("Failed to transmit on UART: %d\r\n", ret);
			return -1;
		}
		data_to_write -= data_written;
		data += data_written;
	}

	return dataLength;
}


/*******************************************************************************/
/*							Interrupt UART									   */
/*******************************************************************************/

/* Private variables */

// UART 1 Buffer Variables
int uart1_rx_it_buffer_length = 0;
int uart1_rx_it_put_idx = 0;
int uart1_rx_it_get_idx = 0;
char uart1_rx_it_buffer[UART_IT_BUFFER_LENGTH];

// UART 2 Buffer Variables
int uart2_rx_it_buffer_length = 0;
int uart2_rx_it_put_idx = 0;
int uart2_rx_it_get_idx = 0;
char uart2_rx_it_buffer[UART_IT_BUFFER_LENGTH];

/**
 * @brief   Mimics the behavior of uart_rx() function, but utilizing the UART interrupt
 * 			buffers. Non-blocking function.
 *
 * @param   huartNum The UART identifier. Ex: huart1 -> 1, huart2 -> 2.
 * @param   dataLength The number of bytes to be read from the UART.
 * @param   data The buffer to store the recieved data in.
 * @retval  The number of bytes read from the UART, or -1 if there was an error.
 */
int uart_rx_it(UART_HandleTypeDef *huart, int dataLength, char *data) {
	// get huartNum
	int huartNum = -1;
	for (int i = 0; i < MAX_NUMBER_UART_HANDLES; i++) {
		if (UARTHandles[i] == huart) {
			huartNum = i;
		}
	}

	/* The amount of bytes still needed to be read. */
	uint16_t data_to_read = dataLength;
	/* The amount of bytes read. */
	uint16_t data_read;

#ifdef UART_DEBUG
  printf("uart_rx() - dataLength: %d\r\n", dataLength);
#endif

	while (data_to_read) {
		if (uart_rx_it_get_length(huartNum) > 0) {
			data_read = uart_rx_it_get(huartNum, data_to_read, data);
			if (data_read == -1) {
				return data_read;
			}
			data_to_read -= data_read;
			data += data_read;
		}
	}

#ifdef UART_DEBUG
  for (ret = 0; ret < dataLength; ++ret) {
    printf("%02X", (data - dataLength)[ret]);
  }

  printf("\r\n");
#endif

	return dataLength;
}

/**
 * @brief   Used in the UART interrupt callback to add newly received data to the UART buffer. Once the data
 * 			is in the UART buffer, it can be consumed using the uart_rx_it() function.
 *
 * @param   huartNum The UART identifier. Ex: huart1 -> 1, huart2 -> 2.
 * @param   dataLength The number of bytes received.
 * @param   data Pointer to the array containing the received data.
 * @retval  Returns -1 if error, otherwise returns the number of bytes that were added
 * 			to the UART buffer.
 */
int uart_rx_it_put(int huartNum, int dataLength, char *data) {
	// Check which huart
	if (huartNum == 1) {
		// copy data to rx circular buffer
		for (int i = 0; i < dataLength; i++) {
			// check if buffer is full before adding more
			if (uart1_rx_it_buffer_length == UART_IT_BUFFER_LENGTH) {
				// return number of bytes actually added to buffer
				return i;
			}

			uart1_rx_it_buffer[uart1_rx_it_put_idx] = data[i];
			uart1_rx_it_put_idx++;
			uart1_rx_it_buffer_length++;

			// reached end of circular buffer, go back to beginning
			if (uart1_rx_it_put_idx == UART_IT_BUFFER_LENGTH) {
				uart1_rx_it_put_idx = 0;
			}
		}
	} else if (huartNum == 2) {
		// copy data to rx circular buffer
		for (int i = 0; i < dataLength; i++) {
			// check if buffer is full before adding more
			if (uart2_rx_it_buffer_length == UART_IT_BUFFER_LENGTH) {
				// return number of bytes actually added to buffer
				return i;
			}

			uart2_rx_it_buffer[uart2_rx_it_put_idx] = data[i];
			uart2_rx_it_put_idx++;
			uart2_rx_it_buffer_length++;

			// reached end of circular buffer, go back to beginning
			if (uart2_rx_it_put_idx == UART_IT_BUFFER_LENGTH) {
				uart2_rx_it_put_idx = 0;
			}
		}
	}
	return dataLength;
}

/**
 * @brief   Used in the uart_rx_it() function to consume data from the UART buffer.
 *
 * @param   huartNum The UART identifier. Ex: huart1 -> 1, huart2 -> 2.
 * @param   dataLength The number of bytes of data that will be consumed from the UART buffer.
 * @param	data The pointer to the char buffer where the received data will be written to.
 * @retval  Returns -1 if error, otherwise returns the number of bytes that were
 * 			actually read from the UART buffer.
 */
int uart_rx_it_get(int huartNum, int dataLength, char *data) {
	// Check which huart
	if (huartNum == 1) {
		for (int i = 0; i < dataLength; i++) {
			// check that buffer has any data to read
			if (uart1_rx_it_buffer_length == 0) {
				// return number of bytes actually read from buffer
				return i;
			}

			data[i] = uart1_rx_it_buffer[uart1_rx_it_get_idx];
			uart1_rx_it_get_idx++;
			uart1_rx_it_buffer_length--;

			// reached end of circular buffer, go back to beginning
			if (uart1_rx_it_put_idx == UART_IT_BUFFER_LENGTH) {
				uart1_rx_it_put_idx = 0;
			}
		}
	} else if (huartNum == 2) {
		for (int i = 0; i < dataLength; i++) {
			// check that buffer has any data to read
			if (uart2_rx_it_buffer_length == 0) {
				// return number of bytes actually read from buffer
				return i;
			}

			data[i] = uart2_rx_it_buffer[uart2_rx_it_get_idx];
			uart2_rx_it_get_idx++;
			uart2_rx_it_buffer_length--;

			// reached end of circular buffer, go back to beginning
			if (uart2_rx_it_put_idx == UART_IT_BUFFER_LENGTH) {
				uart2_rx_it_put_idx = 0;
			}
		}
	}

	return dataLength;
}

/**
 * @brief   Returns the current length of the UART receive buffer for the specified UART handle. The length of the buffer
 * 			is the number of bytes that have been received but have not yet been processed by the program. They can be
 * 			obtained using the uart_rx_it() function.
 *
 * @param   huartNum The UART identifier. Ex: huart1 -> 1, huart2 -> 2.
 * @retval  The current length of the UART RX buffer.
 */
int uart_rx_it_get_length(int huartNum) {
	if (huartNum == 1) {
		return uart1_rx_it_buffer_length;
	} else if (huartNum == 2) {
		return uart2_rx_it_buffer_length;
	}
	return -1;
}

/**
 * @brief   Clear all received bytes from the UART buffer.
 * @note    Does not technically clear the buffer, just resets put idx, get idx and length to 0. This will cause newly received
 * 			data to overwrite the old data, essentially clearing the buffer.
 *
 * @param   huartNum The UART identifier. Ex: huart1 -> 1, huart2 -> 2
 * @retval  void
 */
void uart_rx_it_clear_buffer(int huartNum) {
	if (huartNum == 1) {
		uart1_rx_it_buffer_length = 0;
		uart1_rx_it_put_idx = 0;
		uart1_rx_it_get_idx = 0;
	}
	else if (huartNum == 2) {
		uart2_rx_it_buffer_length = 0;
		uart2_rx_it_put_idx = 0;
		uart2_rx_it_get_idx = 0;
	}
}


/*******************************************************************************/
/*							Testing      									   */
/*******************************************************************************/

/**
 * @brief Prints received characters for every 10 characters received. Pretty straighforward.
 *
 * @param   The UART to test.
 * @retval  void
 */
void testUARTInterruptBuffer_Test1(int huartNum) {
	while (1) {
		if (uart_rx_it_get_length(1) >= 10) {
			char buffer[50];
			int rxLength = uart_rx_it_get(1, 10, buffer);
			printf("\nRx: '%.*s'\n", rxLength, buffer);
		}
	}
}

/**
 * @brief Tests behavior of uart_rx_it(). Should be identical to behavior or uart_rx().
 *
 * @param   The UART to test.
 * @retval  void
 */
void testUARTInterruptBuffer_Test2(UART_HandleTypeDef *huart) {
	char buffer[50];

	while (1) {
		int rxLength = uart_rx_it(huart, 10, buffer);
		printf("\nRx: '%.*s'\n", rxLength, buffer);
	}
}
