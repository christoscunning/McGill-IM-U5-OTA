/**
 * @brief   Sample code from Prof. Zilic. Not used in project at all. Just for reference.
 */

#include "dumo_bglib.h"
#include "stm32u5xx_hal.h"
#include <string.h>

UART_HandleTypeDef huart12;
UART_HandleTypeDef hlpuart1;
#define STORAGE_SIZE 0x8000

char flashROM[300000];

// Dummy function prototpyes
int flash_firmwareprocess(void);
void scrubFirmwareFlash(void);
void firmwareupdate(void);
int validate_fw(uint8_t* flashROM, int storageSize, int somethingElse);
int uart_rx(int x, unsigned char* buffer);
void Error_Handler(void);

int mainF() {

// Assigns a platform-specific function to BGLib’s internal function pointers for UART communication.
	//BGLIB_INITIALIZE(on_message_send);

	// Check the presence of potential firmware in flash
	int r = flash_firmwareprocess();
	uint8_t recUSB[177 + 24] = "firmware programming\r\n";
	// Command to upload firmware if found.
	//HAL_UART_Transmit(&huart1,recUSB,21,10);

	if (r == 1) //should reprogram on r==1, set 2 to ignore programming (i.e., if BT121 broken)
			{
#if PRINTF
                  printf(" Firmware found in flash.\r\n");
                  #endif
		HAL_UART_Transmit(&huart12, recUSB, 21, 10);
		//rotating three lights
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_Delay(500);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		firmwareupdate();
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_Delay(400);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
		scrubFirmwareFlash();
	}

	return 0;
}

//and the check is simple:

/**
 * @brief Flash check for potential firmware
 * @param None
 * @retval None
 */
int flash_firmwareprocess(void) {

#if PRINTF
            printf("Checking flash for firmware . . .\r\n");
      #endif

	// Check flash[0] for FC (FF: no firmware, FC: maybe firmware)

	if (flashROM[0] == 0xFC) {

#if PRINTF
            printf("Potential firmware found. Validating . . .\r\n");
            #endif
		return 1;

	} else if (flashROM[0] != 0xFC) {
		HAL_Delay(1000);
#if PRINTF
                  printf("No firmware found in flash.\r\nChecking flash for firmware . . .\r\n");
            #endif

		return 0;
	}

	return 0;
}

/**
 * @brief Firmware update procedure
 * @param None
 * @retval None
 */
void firmwareupdate(void) {

	// Variable for storing function return values.
	int ret;
	// Pointer to cmd packet
	struct dumo_cmd_packet *pck;
	// Buffer for storing data from the serial port
	static unsigned char buffer[BGLIB_MSG_MAXLEN];
	// Length of message payload data.
	uint16_t msg_length;
	// Flag for infinite loop successfully until firmware uploads
	uint8_t firmware_flag = 0;
	// Binary
	int boot_version = 0;
	// Custom Bootloader
	int dfu_version = 0;
	// Check the validity of the firmware file stored in ST flash(boot version=2)

	while (boot_version == 0) {
		boot_version = validate_fw((uint8_t*) flashROM, STORAGE_SIZE, 0);
		if (boot_version == 0) {
			HAL_Delay(1000);
#if PRINTF
                        printf("Bad or no firmware found.\r\nValidating . . .\r\n");
                  #endif
		}

	}
#if PRINTF
            printf("Validation - OK\r\nInitiating firmware upgrade . . .\r\n");
      #endif

	// GPIO interrupt pin toggle (ST PC14 -> BT121 PB12)

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_Delay(400);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_UART_Receive(&hlpuart1, buffer, 1, 1000);

	uint32_t total_len = 0;
	uint8_t data[250];
	uint8_t len = 0;

	/* do one more light change */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

	/* Infinite while loop until firmware_flag sets to 1 */

	//HAL_IWDG_Refresh(&hiwdg);

	while (firmware_flag == 0) {

		// Read enough data from serial port for the BGAPI message header on UART.

		ret = uart_rx(1, buffer);
		if (ret < 0) {
			Error_Handler();
		}

		// If first byte is zero skip it to avoid possible De-synchronization due to inherent UART framing error on module reset

		if (buffer[0] == 0)
			ret = uart_rx(BGLIB_MSG_HEADER_LEN, buffer);
		else
			ret = uart_rx(BGLIB_MSG_HEADER_LEN - 1, buffer + 1);
		if (ret < 0) {
			Error_Handler();
		}

		// The buffer now contains the message header. Refer to BGAPI protocol definition for details on packet format.
		msg_length = BGLIB_MSG_LEN(buffer);

		// Read the payload data if required and store it after the header.

		if (msg_length) {
			ret = uart_rx(msg_length, &buffer[BGLIB_MSG_HEADER_LEN]);
			if (ret < 0) {
				Error_Handler();
			}
		}

		// To access the payload part of the message
		pck = BGLIB_MSG(buffer);

		// Condition to check dfu_version and boot_version

		// Uploads bootloader_version if condition satisfies
		if ((dfu_version == 1) && (boot_version > 1)) {
//                      switch (BGLIB_MSG_ID(buffer))
//                      {
//                                  case dumo_rsp_dfu_flash_set_address_id:
//                                              // Fallthrough
//                                  case dumo_rsp_dfu_flash_upload_id:
//                                              if (pos == bootdfu_size - BT_FLASH_PAGE_SIZE)    // Skip to last code flash page to write bootdfu metadata
//                                              {
//                                                          dumo_cmd_dfu_flash_set_address(FLASH_END - BT_FLASH_PAGE_SIZE);
//                                                          pos++;
//                                                          break;
//                                              }

//                                              printf("*");

//                                              len = fread(data, 1, 128, bootfp);
//                                              total_len += len;
//                                              pos += len;
//                                              if (len)
//                                              {
//                                                          dumo_cmd_dfu_flash_upload(len, data);
//                                              }
//                                              else
//                                              {
//                                                          dumo_cmd_dfu_flash_upload_finish();
//                                              }
//                                              break;
//                                  case dumo_rsp_dfu_flash_upload_finish_id:

//                                              printf("\nbootloader update finished\n");

//                                              dumo_cmd_dfu_reset(0);
//                                              // if (no_reset_flag || no_boot_wait_flag) exit(0);
//                                              break;
//                                  default:
//                                              break;
//                      }
		}

		// Uploads new/edited firmware that is already been stored in ST flash

		else if ((dfu_version > 1) || (boot_version == 1)) {
			switch (BGLIB_MSG_ID(buffer)) {
			// Used to define the starting address on the flash to where the new firmware will be written in.

			case dumo_rsp_dfu_flash_set_address_id:

				// Command used to upload the whole firmware image file in to the BT121 module.

			case dumo_rsp_dfu_flash_upload_id:

				//upload till 250 bytes
				if ((total_len + 250) < STORAGE_SIZE)
					len = 250;
				else
					len = STORAGE_SIZE - total_len;

				if (total_len < STORAGE_SIZE)
					memcpy(data, &flashROM[total_len], len);

				total_len += len;

				if (len) {
					dumo_cmd_dfu_flash_upload(len, data);
				} else {
					dumo_cmd_dfu_flash_upload_finish();
				}
				break;

				//Command used to tell to the device that the DFU file has been fully uploaded successfully.

			case dumo_rsp_dfu_flash_upload_finish_id:

				// Command used to reset the system to normal mode.

				dumo_cmd_dfu_reset(0);

				// Sets firmware_flag to 1 after successful firmware upload to come out of infinite loop.

				firmware_flag = 1;

#if PRINTF
                                                      printf("\r\nFirmware update - OK\r\nRebooting . . .\r\n");
                                    #endif
				break;
			default:
				break;
			}
		} else if (dfu_version > 0) {
#if PRINTF
                        printf("Module bootloader / firmware version mismatch %d/%d\r\n", boot_version, dfu_version);
                  #endif
		}

		// Handle common cases to get bootloader_version and BT121 module address

		switch (BGLIB_MSG_ID(buffer)) {
		//check if packet is event system initialize
		case dumo_evt_system_initialized_id:
#if PRINTF
                                          printf("System Init - OK\r\nBT Address: ");
                                          print_address(pck->evt_system_initialized.address);
                                          printf("\r\n");
                                    #endif
			while (1) {
			}
			//check if packet is event system boot
		case dumo_evt_system_boot_id:
#if PRINTF
                                          printf("System Boot - OK\r\n");
                                    #endif
			break;
			//check if packet is event dfu boot
		case dumo_evt_dfu_boot_id:
			dfu_version = pck->evt_dfu_boot.version;
#if PRINTF
                                          printf("DFU Boot - OK\r\nDFU Version %d\r\n", dfu_version);
                                    #endif
			if ((dfu_version == 1) && (boot_version > 1)) {
				Error_Handler();
				//dumo_cmd_dfu_flash_set_address(BT_BOOTDFU_ADDRESS);
			} else {
				dumo_cmd_dfu_flash_set_address(0);
			}

			total_len = 0;
			break;
		default:
			if (dfu_version == 0) {
				// Command used to reset the system to DFU mode.

				dumo_cmd_dfu_reset(1);
			}
			break;
		}

	}
}
