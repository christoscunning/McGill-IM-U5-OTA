/**
  ******************************************************************************
  * @file           ota.c
  * @brief          OTA functions
  ******************************************************************************
*/

/* Includes */
#include <stdio.h>
#include <string.h>
#include "ota.h"
#include "uart.h"
#include "flash.h"
#include "util.h"


#include "bgapi.h"

/* Private variables */

/* Functions */

/**
 * @deprecated
 *
 * Downloads firmware data from UART and stores it in the firmware variable.
 * Returns 0 if successful, 1 if error occurred.
 */
int download_firmware(UART_HandleTypeDef *huart, char *firmware, int size) {
	int chunkSize = 8192;

	// download firmware in page sized chunks (8 Kbytes)
	int numChunks = size / chunkSize;
	int leftOverBytes = size - (chunkSize * numChunks);
	printf("downloading %d chunks of %d bytes each + %d left over bytes.\n", numChunks, chunkSize, leftOverBytes);

	char firmwareChunk[8192];
	//int flashWriteAddr = FLASH_FIRMWARE_STARTING_ADDR;

	// get chunk from uart and write to flash
	for (int i = 0; i < numChunks; i++) {
		// read firmware data from uart
		//int ret = uart_rx(huart, chunkSize, firmwareChunk);
		HAL_StatusTypeDef ret = HAL_UART_Receive(huart,(uint8_t *) firmwareChunk, chunkSize, HAL_MAX_DELAY);
		if (ret != HAL_OK) {
			printf("failure reading firmware data over uart. ret = %d\n", ret);
			return 1;
		}

		// save firmware data
		memcpy(firmware, firmwareChunk, chunkSize);

		firmware += chunkSize;
		//flashWriteAddr += 8192;

		printf("downloaded chunk, sending confirmation: \n");

		// signal ready for more:
		char confirmation[] = {0xFF};
		uart_tx(huart, 1, confirmation);
	}



	return 0;
}

/**
 * Receives the 32 byte digest of the original firmware file over UART connection.
 */
HAL_StatusTypeDef downloadFirmwareHash(UART_HandleTypeDef *huart, char *digest) {
	HAL_StatusTypeDef status = HAL_UART_Receive(huart, (uint8_t *) digest, 32, HAL_MAX_DELAY);
	if (status != HAL_OK) {
		printf("Error: failed to receive firmware hash digest over UART (status = %d)\n", status);
		return status;
	}
	return HAL_OK;
}

/**
 * Checks if the computed digest of the received file matches the digest of the original firmware data.
 *
 * @param originalDigest Digest received over UART of the original firmware data.
 * @param computedDigest Digest computed based on the received firmware data.
 * @retval HAL_OK if the digests match (i.e. firmware matches) or HAL_ERROR if they do not match.
 */
HAL_StatusTypeDef checkFirmwareHash(char *originalDigest, char *computedDigest) {
	for (int i = 0; i < 32; i++) {
		if (originalDigest[i] != computedDigest[i]) {
			return HAL_ERROR;
		}
	}
	return HAL_OK;
}

/**
 * Download firmware over UART, and store it in flash memory.
 *
 * @param   huart         The UART handle that will be used to receive firmware data.
 * @param   flashAddress  The starting address of where to put firmware in flash. Must be an address corresponding to the start of a flash page.
 * @param   flashBank     The flash bank that the firmware should be written too.
 * @param   size          The size of the firmware in bytes that will be downloaded over UART.
 * @retval  Status code indicating success or failure of firmware download.
 */
HAL_StatusTypeDef downloadFirmwareToFlash(UART_HandleTypeDef *huart, uint32_t flashAddress, int size) {
	if (flashAddress % FLASH_PAGE_SIZE != 0) {
		printf("Error: input parameter 'flashAddress' must be an address corresponding to the start of a flash page (i.e. a multiple of FLASH_PAGE_SIZE).\n");
		return HAL_ERROR;
	}

	// Receive and store to flash 1 page (8KB) at a time
	int numPages = size / FLASH_PAGE_SIZE;
	int leftOverBytes = size - (numPages * FLASH_PAGE_SIZE);
	printf("Downloading %d pages of %d bytes each + %d left over bytes to flash.\n", numPages, FLASH_PAGE_SIZE, leftOverBytes);

	char firmwarePage[FLASH_PAGE_SIZE];


	uint32_t flashPage = (flashAddress - 0x08000000) / FLASH_PAGE_SIZE; // starting page

	for (int i = 0; i < numPages; i++) {
		// received FLASH_PAGE_SIZE bytes over uart
		while (uart_rx_it_get_length(get_UART_num(huart)) != FLASH_PAGE_SIZE) {
			// waiting for all FLASH_PAGE_SIZE bytes to be received...
		}
//		HAL_	StatusTypeDef status = HAL_UART_Receive(huart, (uint8_t *) firmwarePage, FLASH_PAGE_SIZE, HAL_MAX_DELAY);
//		if (status != HAL_OK) {
//			printf("Error: failed to receive firmware data (page = %d) over UART (status = %d)\n", i, status);
//			return status;
//		}
		if (uart_rx_it(huart, FLASH_PAGE_SIZE, firmwarePage) != FLASH_PAGE_SIZE) {
			// did not received correct amount of bytes
			printf("Error: did not receive correct amount of bytes.\n");
			return HAL_ERROR;
		}

		uart_rx_it_clear_buffer(get_UART_num(huart));

		// write firmware page to flash
		if (eraseFlashPage_Advanced(flashPage+i) != HAL_OK) {
			return HAL_ERROR;
		}
		if (writeFlashPage(flashPage+i, firmwarePage) != HAL_OK) {
			return HAL_ERROR;
		}

		// with advance erase and write functions
//		uint32_t flashEraseBank;
//		uint32_t flashErasePage;
//		if (areFlashBanksSwapped() == 0) { // flash banks not swapped
//			if (flashAddress < 0x08200000) {
//				// bank 1
//				flashEraseBank = FLASH_BANK_1;
//				flashErasePage = flashPage + i;
//			} else {
//				// bank 2
//				flashEraseBank = FLASH_BANK_2;
//				flashErasePage = flashPage + i - 256;
//			}
//		} else { // flash banks are swapped
//			if (flashAddress < 0x08200000) {
//				// bank 2
//				flashEraseBank = FLASH_BANK_2;
//				flashErasePage = flashPage + i;
//			} else {
//				// bank 1
//				flashEraseBank = FLASH_BANK_1;
//				flashErasePage = flashPage + i - 256;
//			}
//		}
//		if (eraseFlashPage_Advanced(flashEraseBank, flashErasePage, 1) != HAL_OK) {
//			return HAL_ERROR;
//		}
//		if (writeFlashPage(flashPage+i, firmwarePage) != HAL_OK) {
//			return HAL_ERROR;
//		}


		printf("Downloaded page %d, sending confirmation:\n", i);

		// send confirmation signal to receive next page
		char confirmation[] = {0xFF};
		uart_tx(huart, 1, confirmation);
	}

	// receive left over bytes
	if (leftOverBytes != 0) {
		// clear firmwarePage buffer
		memset((void *) firmwarePage, 255, FLASH_PAGE_SIZE);

		while (uart_rx_it_get_length(get_UART_num(huart)) != leftOverBytes) {
			// wait until all left over bytes have been received
		}
		if (uart_rx_it(huart, leftOverBytes, firmwarePage) != leftOverBytes) {
			printf("Error: did not receive correct amount of bytes.\n");
			return HAL_ERROR;
		}

		uart_rx_it_clear_buffer(get_UART_num(huart));


		// write received data to flash
		if (eraseFlashPage_Advanced(flashPage + numPages) != HAL_OK) {
			return HAL_ERROR;
		}
		if (writeFlashPage(flashPage + numPages, firmwarePage) != HAL_OK) {
			return HAL_ERROR;
		}

		printf("Downloaded extra bytes. Sending confirmation:\n");

		char confirmation[] = {0xFF};
		uart_tx(huart, 1, confirmation);

	}


	return HAL_OK;
}


/**
 * @brief   Once firmware has been downloaded to flash, use this function to upload it to the BT122 device,
 * 			and finish the firmware upgrade using BGAPI.
 *
 * @param   flashAddress The start address in flash where the firmware data is stored.
 * @param   firmwareSize The size of the firmware data in bytes.
 * @retval
 */
FirmwareInfo uploadFirmwareToBT122(UART_HandleTypeDef *huart, const uint32_t flashAddress, const uint32_t firmwareSize) {
	//update firmware using BGAPI

	// set BT122 UART mode to BGAPI mode
	setBT122UARTMode(BGAPI_MODE);

	// Variable for storing function return values.
	int ret;
	// Pointer to cmd packet
	struct dumo_cmd_packet *pck;
	// Buffer for storing data from the serial port
	static char bg_buffer[BGLIB_MSG_MAXLEN];
	// Length of message payload data.
	uint16_t msg_length;
	// flag for while loop
	int firmwareFlag = 1;
	// keep track of how many bytes of firmware we have written
	int firmwareBytesWritten = 0;

	// FirmwareInfo return variable
	FirmwareInfo fi;
	fi.oldBootloaderVersion = 0;
	fi.major = 0;
	fi.minor = 0;
	fi.patch = 0;
	fi.build = 0;
	fi.newBootloaderVersion = 0;
	fi.hardwareType = 0;

	// start firmware upgrade process by booting into DFU mode (1)
	//dumo_cmd_system_reset((uint8_t) 1);
	dumo_cmd_dfu_reset((uint8_t) 1);

	while (firmwareFlag) {
		// Read enough data from UART to get BGAPI message header
		ret = uart_rx_it(huart, 1, bg_buffer);
		if (ret < 0) {
			//Error_Handler();
			fi.status = HAL_ERROR;
			return fi;
		}

		// If first byte is zero skip it to avoid possible De-synchronization due to inherent UART framing error on module reset
		if (bg_buffer[0] == 0) {
			ret = uart_rx_it(huart, BGLIB_MSG_HEADER_LEN, bg_buffer);
		} else {
			ret = uart_rx_it(huart, BGLIB_MSG_HEADER_LEN - 1, bg_buffer + 1);
		}
		if (ret < 0) {
			//Error_Handler();
			fi.status = HAL_ERROR;
			return fi;
		}

		// The buffer now contains the message header. Refer to BGAPI protocol definition for details on packet format.
		msg_length = BGLIB_MSG_LEN(bg_buffer);
		// Read the payload data if required and store it after the header.
		if (msg_length) {
			ret = uart_rx_it(huart, msg_length, &bg_buffer[BGLIB_MSG_HEADER_LEN]);
			if (ret < 0) {
				//Error_Handler();
				fi.status = HAL_ERROR;
				return fi;
			}
		}
		// To access the payload part of the message
		pck = BGLIB_MSG(bg_buffer);

		// upload new firmware
		switch (BGLIB_MSG_ID(bg_buffer)) {
		case dumo_evt_dfu_boot_id:
			// if we boot into dfu mode after writing all flash data, then that means something was wrong with firmware image
			if (firmwareBytesWritten == firmwareSize) {
				printf("Error: problem with firmware image.. failed to boot with new firmware.\n");
				dumo_cmd_dfu_reset(0);
				fi.status = HAL_ERROR;
				return fi;
			}

			// This event is triggered when device is booted into DFU mode.
			fi.oldBootloaderVersion = pck->evt_dfu_boot.version;

			printf("Booted into DFU mode: version = %ld\n", fi.oldBootloaderVersion);

			// After re-booting device into DFU mode, start flash upload process by defining starting address.
			// When uploading firmware + bootloader, value 0x00000000 should be used
			dumo_cmd_dfu_flash_set_address(0x00000000);

			break;
		case dumo_rsp_dfu_flash_set_address_id:
			// Used to define the starting address on the flash to where the new firmware will be written in.

			// Check result code (0: success, non-zero: error occurred)
			ret = pck->rsp_dfu_flash_set_address.result;
			if (ret == 0) {
				printf("dfu_flash_set_address: Success\n");
			} else {
				printf("dfu_flash_set_address: Error\n");
				fi.status = HAL_ERROR;
				return fi;
			}

			// Fall-through to next case if dfu_flash_set_address was successful.

		case dumo_rsp_dfu_flash_upload_id:
			// check result code of previous flash upload (if not a fall-through)
			if (BGLIB_MSG_ID(bg_buffer) == dumo_rsp_dfu_flash_upload_id) {
				ret = pck->rsp_dfu_flash_upload.result;
				if (ret == 0) {
					//printf("dfu_flash_upload %d: Success\n", firmware_bytes_written);
				} else {
					printf("dfu_flash_upload %d: Error\n", firmwareBytesWritten);
					fi.status = HAL_ERROR;
					return fi;
				}
			}

			// check how much firmware has been written to BT122
			if (firmwareBytesWritten == firmwareSize) {
				// if we have written all firmware bytes, go to flash_upload_finish
				dumo_cmd_dfu_flash_upload_finish();
			} else {
				// else keep writing firmware data to BT122
				// upload 128 bytes at a time
				char firmwareChunk[128];
				memcpy(firmwareChunk, (void *) flashAddress + firmwareBytesWritten, 128);

				dumo_cmd_dfu_flash_upload(128, firmwareChunk);
				firmwareBytesWritten += 128;

				// Print updates on progress
				if (firmwareBytesWritten % 8192 == 0) {
					printf("Written %d / %ld bytes.\n", firmwareBytesWritten, firmwareSize);
				}
			}

			break;
		case dumo_rsp_dfu_flash_upload_finish_id:
			//Command used to tell to the device that the DFU file has been fully uploaded successfully.

			// check result code of dfu_flash_upload_finish
			ret = pck->rsp_dfu_flash_upload_finish.result;
			if (ret == 0) {
				printf("dfu_flash_upload_finish: Success\n");
			} else {
				printf("dfu_flash_upload_finish: Error\n");
				fi.status = HAL_ERROR;
				return fi;
			}

			// Command used to reset the system to normal mode.
			printf("\r\nFirmware upload - OK -> Rebooting . . .\n");
			dumo_cmd_dfu_reset(0);

			break;
		case dumo_evt_system_boot_id:
			// This event is triggered when device boots into normal mode
			fi.major = pck->evt_system_boot.major;
			fi.minor = pck->evt_system_boot.minor;
			fi.patch = pck->evt_system_boot.patch;
			fi.build = pck->evt_system_boot.build;
			fi.newBootloaderVersion = pck->evt_system_boot.bootloader;
			fi.hardwareType = pck->evt_system_boot.hw;

			fi.status = HAL_OK;

			// Sets firmware_flag to 0 after successful firmware upload to come out of infinite loop.
			firmwareFlag = 0;

			break;
		default:
			printf("BGAPI response does match an expected rsp/evt.  unknown ID = %ld\n", BGLIB_MSG_ID(bg_buffer));
			break;
		}
	}

	printf("BT122 firmware upgrade procedure complete.\n");

	return fi;
}
