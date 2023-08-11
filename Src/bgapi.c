/**
  ******************************************************************************
  * @file           bgapi.c
  * @brief          BGAPI functions
  ******************************************************************************
*/

/* Includes */
#include <stdio.h>
#include <string.h>
#include "bgapi.h"

#include "main.h"
#include "uart.h"
#include "errno.h"
#include "util.h"

/**
 * Define BGLIB library
 */
BGLIB_DEFINE();

/* Static functions prototype */
static void onMessageSend(uint8_t msg_len, uint8_t *msg_data, uint16_t data_len, uint8_t *data);


/* Private variables */

// UART handle used for BGAPI communication with BT122
UART_HandleTypeDef *huartBGAPI;


/* Functions */

/**
 * Initializing the BGAPI library. Also gets the uart handle that will be used for
 * communicating with the BT122.
 *
 * @param   huart The handle of the UART connected to BT122.
 */
void initializeBGLIB(UART_HandleTypeDef *huart) {
	// Get pointer to UART handle that will be used for executing bgapi operations
	huartBGAPI = huart;

	/* Initialize BGLIB with our output function for sending messages. */
	BGLIB_INITIALIZE(onMessageSend);
}

/**
 * Function called when a message needs to be written to the serial port.
 *
 * @param msg_len  Length of the message.
 * @param msg_data Message data, including the header.
 * @param data_len Optional variable data length.
 * @param data     Optional variable data.
 */
static void onMessageSend(uint8_t msg_len, uint8_t *msg_data, uint16_t data_len, uint8_t *data) {
	/* Variable for storing function return values. */
	int ret;

	ret = uart_tx(huartBGAPI, msg_len, (char*) msg_data);
	if (ret < 0) {
		printf("onMessageSend() - failed to write to serial port, ret: %d, errno: %d\r\n", ret,
		errno);
		return;
	}

	if (data_len && data) {
		ret = uart_tx(huartBGAPI, data_len, (char*) data);

		if (ret < 0) {
			printf("onMessageSend() - failed to write to serial port, ret: %d, errno: %d\r\n", ret,
					errno);
			return;
		}
	}
}

/**
* @brief   Print MAC address.
* @param   bd_addr MAC address to print.
*/
void printMACAddress(bd_addr address) {
	for (int i = 5; i >= 0; i--) {
		printf("%02x", address.addr[i]);

		if (i > 0) {
			printf(":");
		}
	}
}

void echoReceived(uint8_t endpoint, unsigned int bytes) {
	char tmp[100];
	sprintf(tmp, "Received: %u\r\n", bytes);
	dumo_cmd_endpoint_send(endpoint, strlen(tmp), tmp);
}



/**
 * Turn BT122 UART to data mode or BGAPI mode. Note, LED0 on bt122 indicates UART mode:
 * LED0: on (0) = BGAPI mode, off (1) = DATA mode.
 * Note: LED0 is active low.
 *
 * @param   mode 0: set BT122 UART in BGAPI mode
 * 			     1: set BT122 UART in DATA  mode
 * @retval  HAL_OK if BT122 UART mode set succesfully, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef setBT122UARTMode(int mode) {
	int currentMode = HAL_GPIO_ReadPin(BT122_PF3_GPIO_Port, BT122_PF3_Pin);

	if (mode == 0) {
		// set BT122 UART to BGAPI mode
		if (currentMode == 0) {
			// already in BGAPI mode
			printf("Already in BGAPI mode\n");
		} else {
			printf("Switching to BGAPI mode\n");
			// Toggle PF2 pin to trigger interrupt
			HAL_GPIO_WritePin(BT122_PF2_GPIO_Port, BT122_PF2_Pin, GPIO_PIN_RESET);
			HAL_Delay(1);
			HAL_GPIO_WritePin(BT122_PF2_GPIO_Port, BT122_PF2_Pin, GPIO_PIN_SET);
		}
	} else if (mode == 1) {
		if (currentMode == 1) {
			// already in data mode
			printf("Already in data mode\n");
		} else {
			printf("Switching to data mode\n");
			// Toggle PF2 pin to trigger interrupt
			HAL_GPIO_WritePin(BT122_PF2_GPIO_Port, BT122_PF2_Pin, GPIO_PIN_RESET);
			HAL_Delay(1);
			HAL_GPIO_WritePin(BT122_PF2_GPIO_Port, BT122_PF2_Pin, GPIO_PIN_SET);
		}
	} else {
		printf("Error: mode must be 1 or 0.\n");
		return HAL_ERROR;
	}

	return HAL_OK;
}


/**
 * Test toggling between BT122 UART modes.
 */
void testBGAPI_Test1() {
	printf("\n\n***************************************\nBGAPI TEST 1 - Toggle BT122 UART Mode test\n***************************************\n\n");

	// try toggling GPIO->BT122_PF2
	setBT122UARTMode(0); // set to bgapi mode
	HAL_Delay(1000);
	setBT122UARTMode(1); // set to data mode
	HAL_Delay(1000);
	setBT122UARTMode(0); // set to bgapi mode

	printf("BGAPI - Test 1 complete.\n");
}

/**
 * Test BGAPI operations with BT122. Get BT address of BT122.
 */
void testBGAPI_Test2() {
	printf("\n\n***************************************\nBGAPI TEST 2 - Basic BGAPI functionality (get bt address)\n***************************************\n\n");

	// Make sure bt122 uart is in BGAPI mode
	setBT122UARTMode(BGAPI_MODE);

	// Variable for storing function return values.
	int ret;
	// Pointer to cmd packet
	struct dumo_cmd_packet *pck;
	// Buffer for storing data from the serial port
	static char bg_buffer[BGLIB_MSG_MAXLEN];
	// Length of message payload data.
	uint16_t msg_length;

	dumo_cmd_system_reset(0);

	HAL_Delay(1000);

	dumo_cmd_system_get_bt_address();

	for (int i = 0; i < 5;) {
		// Read enough data from UART to get BGAPI message header
		ret = uart_rx_it(huartBGAPI, 1, bg_buffer);
		if (ret < 0) {
			Error_Handler();
		}

		// If first byte is zero skip it to avoid possible De-synchronization due to inherent UART framing error on module reset
		if (bg_buffer[0] == 0) {
			ret = uart_rx_it(huartBGAPI, BGLIB_MSG_HEADER_LEN, bg_buffer);
		} else {
			ret = uart_rx_it(huartBGAPI, BGLIB_MSG_HEADER_LEN - 1, bg_buffer + 1);
		}
		if (ret < 0) {
			Error_Handler();
		}


		// The buffer now contains the message header. Refer to BGAPI protocol definition for details on packet format.
		msg_length = BGLIB_MSG_LEN(bg_buffer);

		// Read the payload data if required and store it after the header.
		if (msg_length) {
			ret = uart_rx_it(huartBGAPI, msg_length, &bg_buffer[BGLIB_MSG_HEADER_LEN]);
			if (ret < 0) {
				Error_Handler();
			}
		}
		// To access the payload part of the message
		pck = BGLIB_MSG(bg_buffer);

		// do something based on message ID

		if (BGLIB_MSG_ID(bg_buffer) == dumo_rsp_system_get_bt_address_id) {
			bd_addr addr = pck->rsp_system_get_bt_address.address;

			printf("BT Addr: ");
			//printBuffer((char*) addr.addr, 6, "%02x");
			printMACAddress(addr);
			printf("\n");

			dumo_cmd_system_get_bt_address();

			i++;
		}
	}
	printf("BGAPI - Test 2 complete.\n");
}

/**
 * Test system reset events.
 */
void testBGAPI_Test3() {
	printf("\n\n***************************************\nBGAPI TEST 3 - System Reset (normal vs dfu modes)\n***************************************\n\n");

	// Make sure bt122 uart is in BGAPI mode
	setBT122UARTMode(BGAPI_MODE);

	// Variable for storing function return values.
	int ret;
	// Pointer to cmd packet
	struct dumo_cmd_packet *pck;
	// Buffer for storing data from the serial port
	static char bg_buffer[BGLIB_MSG_MAXLEN];
	// Length of message payload data.
	uint16_t msg_length;

	// loop flag
	int firstFlag = 1;
	int secondFlag = 1;

	printf("Reseting bt122 into normal mode\n");
	dumo_cmd_system_reset(0);

	while (secondFlag) {
		// Read enough data from UART to get BGAPI message header
		ret = uart_rx_it(huartBGAPI, 1, bg_buffer);
		if (ret < 0) {
			Error_Handler();
		}

		// If first byte is zero skip it to avoid possible De-synchronization due to inherent UART framing error on module reset
		if (bg_buffer[0] == 0) {
			ret = uart_rx_it(huartBGAPI, BGLIB_MSG_HEADER_LEN, bg_buffer);
		} else {
			ret = uart_rx_it(huartBGAPI, BGLIB_MSG_HEADER_LEN - 1, bg_buffer + 1);
		}
		if (ret < 0) {
			Error_Handler();
		}

		// The buffer now contains the message header. Refer to BGAPI protocol definition for details on packet format.
		msg_length = BGLIB_MSG_LEN(bg_buffer);


		// print bgapi packet
		printf("BGAPI packet: ");
		printBuffer(bg_buffer, msg_length, "%02x");

		// Read the payload data if required and store it after the header.
		if (msg_length) {
			ret = uart_rx_it(huartBGAPI, msg_length, &bg_buffer[BGLIB_MSG_HEADER_LEN]);
			if (ret < 0) {
				Error_Handler();
			}
		}
		// To access the payload part of the message
		pck = BGLIB_MSG(bg_buffer);

		// do something based on message ID
		switch (BGLIB_MSG_ID(bg_buffer)) {
			case dumo_evt_system_initialized_id:
				printf("dumo_evt_system_initialized ");
				printMACAddress(pck->evt_system_initialized.address);
				printf("\n");

				// if already booted to dfu mode once (firstFlag == 0), then exit loop
				if (firstFlag == 0) {
					printf("Second time booting into normal mode, exiting test...\n");
					secondFlag = 0;
				} else {
					// boot to dfu mode
					printf("First time booting into normal mode, next booting into dfu mode...\n");
					dumo_cmd_system_reset(1);
				}
				break;
			case dumo_evt_system_boot_id:
				printf("(normal) System Boot\n");
				break;
			case dumo_evt_dfu_boot_id:
				;
				uint32_t dfu_version = pck->evt_dfu_boot.version;
				printf("DFU boot, DFU version %ld\n", dfu_version);

				// go back to normal mode
				dumo_cmd_dfu_reset(0);

				// exit loop
				firstFlag = 0;
				break;
			default:
				break;
		}
	}

	printf("BGAPI - Test 3 complete.\n");
}

