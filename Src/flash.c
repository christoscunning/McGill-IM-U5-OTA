/**
 ******************************************************************************
 * @file           flash.c
 * @brief          FLASH functions
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "flash.h"
#include "util.h"

/* Private variables --------------------------------------------------------*/

/* Functions ----------------------------------------------------------------*/

/**
 * @brief   Print the flash error description based off of flash error code.
 *
 * @param   flashErrorCode The error code returned from HAL_FLASH_GetError().
 */
void printFlashError(uint32_t flashErrorCode) {
	printf("Flash Error Code: %d => ", (int) flashErrorCode);
	if ((flashErrorCode & HAL_FLASH_ERROR_NONE) != 0)
		printf("HAL_FLASH_ERROR_NONE & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_OP) != 0)
		printf("HAL_FLASH_ERROR_OP & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_PROG) != 0)
		printf(" HAL_FLASH_ERROR_PROG & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_WRP) != 0)
		printf("HAL_FLASH_ERROR_WRP & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_PGA) != 0)
		printf("HAL_FLASH_ERROR_PGA & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_SIZ) != 0)
		printf("HAL_FLASH_ERROR_SIZ & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_PGS) != 0)
		printf("HAL_FLASH_ERROR_PGS & ");
	if ((flashErrorCode & HAL_FLASH_ERROR_OPTW) != 0)
		printf("HAL_FLASH_ERROR_OPTW & ");
	printf("\n");
}

/**
 * @brief   Clears all flash error flags.
 */
void clearAllFlashFlags() {
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
}

/**
 * @brief   Query Flash option bytes to determine if flash banks are currently swapped or not.
 *          i.e.: check if bank 1 and bank 2 addresses are swapped.
 * @retval  0 if flash banks are not swapped. 1 if flash banks are swapped.
 */
int areFlashBanksSwapped() {
	// get current option bytes
	FLASH_OBProgramInitTypeDef opbytes;
	HAL_FLASHEx_OBGetConfig(&opbytes);
	//printf("UserConfig OB: 0x%08lx\n", opbytes.USERConfig);

	// current value of OB_SWAP_BANKS bit
	uint32_t swap_banks = opbytes.USERConfig & (0x1 << 20U);
	//printf("Swap banks: 0x%08lx\n", swap_banks);

	if (swap_banks == 0) {
		return 0;
	} else {
		return 1;
	}
}


/**
 * Read the word located at a specified address + offset from flash memory.
 *
 * @param baseAddress Base address to use when calculating address to read from.
 * @param offset The offset added to the base address to calculate the address to read from.
 * @return The word (4 bytes) of data the is located at (baseAddress + offset) in flash memory.
 */
uint32_t readFlash(int baseAddress, int offset) {
	__IO uint32_t *basePtr = (uint32_t*) baseAddress;
	return *(basePtr + offset);
}

/**
 * 	Read a page of data from flash, and store it in buffer.
 *
 *	 @param page The page number of data to be read from flash (between 0 and (FLASH_PAGE_NB - 1)).
 * 	@param buffer The buffer in which the read data will be stored. Must have a length of at least FLASH_PAGE_SIZE (8 KB).
 *	@return Status code indicating if read operation was successful or not.
 */
HAL_StatusTypeDef readFlashPage(int page, char *buffer) {
	__IO uint32_t *flashPage = (uint32_t*) (0x08000000 + (page * FLASH_PAGE_SIZE));

	// Shouldn't use memcpy with volatile pointers
	//buffer = memcpy(buffer, flashPage, FLASH_PAGE_SIZE);

	for (int i = 0; i < FLASH_PAGE_SIZE; i++) {
		char byte = ((char*) (flashPage))[i % 4];
		buffer[i] = byte;

		if (i != 0 && i % 4 == 0) {
			flashPage++;
		}
	}

	return HAL_OK;
}

/**
 * 	Prints the data that is in the flash memory from FLASH_START_ADDR to (FLASH_END_ADDR - 4).
 *	Note: does not print the word located at FLASH_END_ADDR.
 *
 * 	@param FLASH_START_ADDR The address specifying the start of block of data to print.
 * 	@param FLASH_END_ADDR Stops printing before reaching this address.
 */
void printFlashData(const uint32_t FLASH_START_ADDR, const uint32_t FLASH_END_ADDR) {
	/* Print data from flash from FLASH_START_ADDR to FLASH_END_ADDR */
	uint32_t Address = FLASH_START_ADDR;

	while (Address < FLASH_END_ADDR) {
		printf("Word @ Addr (%08lx): %08lx\n", Address, (*(__IO uint32_t*) Address));
		Address += 4;
	}
}

/**
 *	Program a quad-word (128-bits / 16 bytes) at a specified address.
 *	!!! This function assumes that the flash memory at the specified address has already been erased !!!
 *
 *	@param flashAddress: specifies the address to be programmed. This parameter shall be aligned to the Flash word (128 bits)
 *	@param dataAddress: specifies the address of data to be programmed. This parameter shall be 32-bit aligned.
 *	@return Status code indicating if write operation was successful or not.
 */
HAL_StatusTypeDef writeFlash_OLD(uint32_t flashAddress, uint32_t dataAddress) {
	// make sure flash address is 128 bit aligned
	if ((flashAddress & 0xffff) != 0) {
		printf("Error: flash address to write to flash must be 128-bit aligned.\n");
		return HAL_ERROR;
	}
	// make sure data address is 32 bit aligned
	if ((dataAddress & 0x3) != 0) {
		printf("Error: data address to write to flash must be 32-bit aligned.\n");
		return HAL_ERROR;
	}

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
		return HAL_ERROR;
	}
	clearAllFlashFlags();
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, flashAddress,dataAddress) != HAL_OK) {
		// Check for error with flash programming operation
		uint32_t flashErrorCode = HAL_FLASH_GetError();
		printf("HAL_FLASH_Program() failed with error code: %d\n", (int) flashErrorCode);
		printFlashError(flashErrorCode);
		return HAL_ERROR;
	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
		return HAL_ERROR;
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef writeFlash(uint32_t flashAddress, uint32_t dataAddress) {
	// make sure flash address is 128 bit aligned
	if ((flashAddress & 0xf) != 0) {
		printf("Error: flash address to write to flash must be 128-bit aligned.\n");
		return HAL_ERROR;
	}
	// make sure data address is 32 bit aligned
	if ((dataAddress & 0x3) != 0) {
		printf("Error: data address to write to flash must be 32-bit aligned.\n");
		return HAL_ERROR;
	}

	HAL_FLASH_Unlock();
//	if (HAL_FLASH_Unlock() != HAL_OK) {
//		printf("Failed to unlock flash\n");
//	}
	clearAllFlashFlags();
	HAL_StatusTypeDef programStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, flashAddress, dataAddress);
	FLASH_WaitForLastOperation(HAL_MAX_DELAY);
//	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
//		printf("Wait failed...\n");
//	}
	HAL_FLASH_Lock();
//	if (HAL_FLASH_Lock() != HAL_OK) {
//		printf("Failed to lock flash\n");
//	}

	return programStatus;
}

/**
 * Assumes page has already been erased.
 *
 * @param page The page number to write too.
 * @param buffer The buffer containing the data to be written to flash. Must be FLASH_PAGE_SIZE # of bytes in length.
 * @return Status code indicated if flash page write was a success or failure.
 */
HAL_StatusTypeDef writeFlashPage(int page, const char *buffer) {
	//char writeBuffer[16] __attribute__ ((aligned(4)));
	uint32_t flashAddress = 0x08000000 + (page * FLASH_PAGE_SIZE);
	uint32_t bufferCursor = (uint32_t) (buffer);
	// write 8192 bytes to flash in sets of 16 bytes
	for (int i = 0; i < FLASH_PAGE_SIZE / 16; i++) {
		//memcpy(writeBuffer, bufferCursor, 16);
		HAL_StatusTypeDef status = writeFlash(flashAddress, bufferCursor);
		if (status != HAL_OK) {
			printf("Page write failed at address: %ld\n", flashAddress);
			return status;;
		}
		flashAddress += 16;
		bufferCursor += 16;
	}

	return HAL_OK;
}

/**
 *	Write a large amount of data to the flash memory.
 *
 *	For now, requires length to be a multiple of 128 bits / 16 bytes (flash page size)
 *
 *	@param flashAddress Address at which to start writing data.
 *	@param buffer The address of the buffer containing the data to write to flash.
 *	@param length The length of the buffer containing the data.
 *	@return Status code indicating if write operation was successful or not.
 */
HAL_StatusTypeDef writeFlashLarge(uint32_t flashAddress, const char *buffer, const int length) {
	if (length % 16 != 0) {
		printf("Error: buffer length must be multiple of 16.\n");
		return HAL_ERROR;
	}

	char writeBuffer[16] __attribute__ ((aligned(4))); // Anything written to flash must be 32-bit aligned
	uint32_t bufferCursor = (uint32_t) (&writeBuffer);

	// write to flash in sets of 128 bits (16 bytes)
	for (int i = 0; i < length / 16; i++) {
		memcpy(writeBuffer, buffer, 16);
		HAL_StatusTypeDef status = writeFlash(flashAddress, bufferCursor);
		if (status != HAL_OK) {
			printf("Error: failed to write %d chunk to flash.\n", i);
			return status;
		}
		flashAddress += 16;
		bufferCursor += 16;
	}

	return HAL_OK;
}

/**
 * Erases the specified page of flash memory.
 *
 * @param page The flash page to erase.
 * @return Status code indicating if erase operation was successful or not.
 */
HAL_StatusTypeDef eraseFlashPage_OLD(int page) {
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Banks = FLASH_BANK_1;
	eraseInit.Page = page;
	eraseInit.NbPages = 1;
	uint32_t eraseError;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
		return HAL_ERROR;
	}
	clearAllFlashFlags();
//	HAL_StatusTypeDef eraseStatus = HAL_FLASHEx_Erase(&eraseInit, &pageError);
//	// Only print status if there is an error.
//	if (eraseStatus != HAL_OK || pageError != 0xffffffff) {
//		printf("HAL Erase Status: %d, Page Error: %08x\n", eraseStatus, (int) pageError);
//		return eraseStatus;
//	}

	if (HAL_FLASHEx_Erase(&eraseInit, &eraseError) != HAL_OK) {
		printf("Failed to erase flash. eraseError = %ld\n", eraseError);
		// This line causes flash operations to fail for some reason....???
		//printf("HAL Erase Status: %ld, Page Error: %08x\n", HAL_FLASH_GetError(), (int) eraseError);
		return HAL_ERROR;
	}

	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
		return HAL_ERROR;
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
		return HAL_ERROR;
	}
	return HAL_OK;
}


/**
 * Erases the specified page of flash memory.
 *
 * @param page The flash page to erase.
 * @return Status code indicating if erase operation was successful or not.
 */
HAL_StatusTypeDef eraseFlashPage(int page) {
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Banks = FLASH_BANK_1;
	eraseInit.Page = page;
	eraseInit.NbPages = 1;
	uint32_t eraseError;
	HAL_FLASH_Unlock();
//	if (HAL_FLASH_Unlock() != HAL_OK) {
//		printf("Failed to unlock flash\n");
//	}
	clearAllFlashFlags();
	HAL_StatusTypeDef eraseStatus = HAL_FLASHEx_Erase(&eraseInit, &eraseError);
	FLASH_WaitForLastOperation(HAL_MAX_DELAY);
//	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
//		printf("Wait failed...\n");
//	}
	HAL_FLASH_Lock();
//	if (HAL_FLASH_Lock() != HAL_OK) {
//		printf("Failed to lock flash\n");
//	}
	return eraseStatus;
}

/**
 * Takes into account potentially swapped banks.
 */
HAL_StatusTypeDef eraseFlashPage_Advanced(int page) {
	// Make sure 'page' parameter is a valid page.
	if (page < 0 || page > FLASH_SIZE / FLASH_PAGE_SIZE) {
		printf("Error: invalid value for parameter 'page'. Must be between 0 and %ld.\n", FLASH_SIZE / FLASH_PAGE_SIZE);
		return HAL_ERROR;
	}
	// Determine what flash bank and page we are really erasing. May not match input 'page'
	// due to swapped banks.
	uint32_t flashEraseBank;
	uint32_t flashErasePage;
	if (areFlashBanksSwapped() == 0) { // flash banks not swapped
		if (page < 256) {
			// bank 1
			flashEraseBank = FLASH_BANK_1;
			flashErasePage = page;
		} else {
			// bank 2
			flashEraseBank = FLASH_BANK_2;
			flashErasePage = page - 256;
		}
	} else { // flash banks are swapped
		if (page < 256) {
			// bank 2
			flashEraseBank = FLASH_BANK_2;
			flashErasePage = page;
		} else {
			// bank 1
			flashEraseBank = FLASH_BANK_1;
			flashErasePage = page - 256;
		}
	}

	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Banks = flashEraseBank;
	eraseInit.Page = flashErasePage;
	eraseInit.NbPages = 1;
	uint32_t eraseError;
	HAL_FLASH_Unlock();
//	if (HAL_FLASH_Unlock() != HAL_OK) {
//		printf("Failed to unlock flash\n");
//	}
	clearAllFlashFlags();
	HAL_StatusTypeDef eraseStatus = HAL_FLASHEx_Erase(&eraseInit, &eraseError);
	FLASH_WaitForLastOperation(HAL_MAX_DELAY);
//	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
//		printf("Wait failed...\n");
//	}
	HAL_FLASH_Lock();
//	if (HAL_FLASH_Lock() != HAL_OK) {
//		printf("Failed to lock flash\n");
//	}
	return eraseStatus;
}


/**
 *	Simplest flash test, use to test most basic functionality of flash functions. Does the following:
 *		1. Erase flash
 *		2. Print flash data
 *		3. Write to flash
 *		4. Print flash data
 *		5. Erase flash
 *		6. Print flash data
 */
void flashTest_1() {
	HAL_StatusTypeDef testStatus = HAL_OK;
	// Test flash simplest case
	printf("\n\n***************************************\nFLASH TEST 1\n***************************************\n\n");

	// Erase page 128
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Banks = FLASH_BANK_1;
	eraseInit.Page = 128;
	eraseInit.NbPages = 1;
	uint32_t eraseError;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();

	// TODO: changed to not check status
	HAL_FLASHEx_Erase(&eraseInit, &eraseError);
//	if (HAL_FLASHEx_Erase(&eraseInit, &eraseError) != HAL_OK) {
//		printf("Failed to erase flash. eraseError = %ld\n", eraseError);
//	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}

	// check that flash was erased
	char expected[FLASH_PAGE_SIZE];
	char actual[FLASH_PAGE_SIZE];
	memset(expected, 255, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, FLASH_PAGE_SIZE);
	}

	printFlashData(0x08100000, 0x08100010);

	// data to write to flash
	char flashData[16] __attribute__ ((aligned(16)));
	memset(flashData, 9, 16);

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();

	// TODO: changed to not check status
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t) 0x08100000, (uint32_t) &flashData);
//	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t) 0x08100000, (uint32_t) &flashData)
//			!= HAL_OK) {
//		printf("Failed to write to flash.\n");
//	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}

	// check that flash was programmed correctly (just first quadword)
	memset(expected, 9, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, 16);
	}

	// read from flash
	//	__IO uint32_t *readAddress = (uint32_t *) 0x08100000;
	//	uint32_t readData = *readAddress;
	//	printf("Reading from flash...  ");
	//	printf("Read: %ld\n", readData);
	//
	//	// test read from flash
	//	for (int i = 0; i < 4; i++) {
	//		printf("%08X\n", (unsigned int) readFlash(0x08100000, i));
	//	}

	printFlashData(0x08100000, 0x08100010);

	// Erase data after reading
	FLASH_EraseInitTypeDef eraseInit2;
	eraseInit2.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit2.Banks = FLASH_BANK_1;
	eraseInit2.Page = 128;
	eraseInit2.NbPages = 1;
	uint32_t eraseError2;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();

	// TODO: Changed to not check status
	HAL_FLASHEx_Erase(&eraseInit2, &eraseError2);
//	if (HAL_FLASHEx_Erase(&eraseInit2, &eraseError2) != HAL_OK) {
//		printf("Failed to erase flash. eraseError = %ld\n", eraseError2);
//	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}
	//flashTest_Erase(128);
	//eraseFlashPage(128);


	// check that flash was erased
	memset(expected, 255, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, FLASH_PAGE_SIZE);
	}

	printFlashData(0x08100000, 0x08100010);

	if (testStatus == HAL_OK) {
		printf("\n\n***************************************\nFLASH TEST 1 - Test Passed\n***************************************\n\n");
	} else if (testStatus == HAL_ERROR) {
		printf("\n\n***************************************\nFLASH TEST 1 - Test Failed\n***************************************\n\n");
	}
}

/**
 *	This flash test uses the functions defined in this file, rather than the HAL functions directly.
 *	If flashTest_1() is working, then the next step is to check if this test is working.
 *
 *	Does the following:
 *		1.  Read flash page to buffer
 *		2.  Erase flash page
 *		3.  Read flash page to buffer
 *		4.  Write flash (quadword)
 *		5.  Read flash page to buffer
 *		6.  Erase flash page
 *		7.  Read flash page to buffer
 *		8.  Write flash page
 *		9.  Read flash page to buffer
 *		10. (optional) Erase flash page
 */
void flashTest_2() {
	HAL_StatusTypeDef testStatus = HAL_OK;
	printf("\n\n***************************************\nFLASH TEST 2\n***************************************\n\n");

	// Read flash data to this buffer
	char flashReadBuffer[FLASH_PAGE_SIZE] __attribute__ ((aligned(16)));
	(void ) flashReadBuffer; // stop unused warning

	// Write data from this buffer to flash
	char flashWriteBuffer[16] __attribute__ ((aligned(16)));

	// For printing
	// Having these uncommented causes test 1 to fail.... why!!!!????
	//const volatile uint32_t flashStartAddr = 135266304; //0x08100000;
	//const volatile uint32_t flashEndAddr   = 135266320; //0x08100010;


	// 1. Read flash page to buffer
	printf("\nReading first 16 bytes of page 128.\n");
//	printf("\nReading page 128.\n");
//	readFlashPage(128, flashReadBuffer);
//
//	// 1.5 print buffer
//	printBuffer(flashReadBuffer, 16, "%02x");
	printFlashData(0x08100000, 0x08100010);


	// 2. Erase flash page
	printf("\nErasing page 128.\n");
	eraseFlashPage(128);


	// check that flash was erased
	char expected[FLASH_PAGE_SIZE];
	char actual[FLASH_PAGE_SIZE];
	memset(expected, 255, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, FLASH_PAGE_SIZE);
	}


	// 3. Read flash page to buffer
	printf("\nReading first 16 bytes of page 128.\n");
//	printf("\nReading page 128.\n");
//	readFlashPage(128, flashReadBuffer);
//
//	// 3.5 print buffer
//	printBuffer(flashReadBuffer, 16, "%02x");
	printFlashData(0x08100000, 0x08100010);

	// 4. Write flash (quadword)
	printf("\nWriting (all 1s) to page 128 (first 16 bytes)\n");
	memset(flashWriteBuffer, 1, 16);
	writeFlash(0x08100000, (uint32_t) &flashWriteBuffer);


	// check that flash was programmed correctly (first quadword ie: 16 bytes)
	memset(expected, 1, 16);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, 16);
	}

	// 5. Read flash page to buffer
	printf("\nReading first 16 bytes of page 128.\n");
//	printf("\nReading page 128.\n");
//	readFlashPage(128, flashReadBuffer);
//
//	// 5.5 print buffer
//	printBuffer(flashReadBuffer, 16, "%02x");
	printFlashData(0x08100000, 0x08100010);


	// 6. Erase flash page
	printf("\nErasing page 128.\n");
	eraseFlashPage(128);


	// check that flash was erased
	memset(expected, 255, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, FLASH_PAGE_SIZE);
	}

	// 7. Read flash page to buffer
	printf("\nReading first 16 bytes of page 128.\n");
//	printf("\nReading page 128.\n");
//	readFlashPage(128, flashReadBuffer);
//
//	// 7.5 print buffer
//	printBuffer(flashReadBuffer, 16, "%02x");
	printFlashData(0x08100000, 0x08100010);

	// 8. Write to flash page
	printf("\nWriting (i*8) to page 128 (first 16 bytes)\n");
	for (int i = 0; i < 16; i++) {
		flashWriteBuffer[i] = i * 8;
	}
	writeFlash(0x08100000, (uint32_t) &flashWriteBuffer);


	// check that flash was programmed correctly - first quadword ie: 16 bytes
	memcpy(expected, flashWriteBuffer, 16);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, 16);
	}

	// 9. Read flash page to buffer
	printf("\nReading first 16 bytes of page 128.\n");
//	printf("\nReading page 128.\n");
//	readFlashPage(128, flashReadBuffer);
//
//	// 9.5 print buffer
//	printBuffer(flashReadBuffer, 16, "%02x");
	printFlashData(0x08100000, 0x08100010);

	// 10. (optional) Erase flash page
	//printf("\n");
	//eraseFlashPage(128);

	if (testStatus == HAL_OK) {
		printf("\n\n***************************************\nFLASH TEST 2 - Test Passed\n***************************************\n\n");
	} else if (testStatus == HAL_ERROR) {
		printf("\n\n***************************************\nFLASH TEST 2 - Test Failed\n***************************************\n\n");
	}
}


/**
 * Pretty similar to flash test 1.
 * 1. Erase page 128
 * 2. Write all 9s to page 128
 * 3. Erase page 128
 */
void flashTest_3() {
	HAL_StatusTypeDef testStatus = HAL_OK;
	// Test flash simplest case
	printf("\n\n***************************************\nFLASH TEST 3\n***************************************\n\n");

	// Erase page 128
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Banks = FLASH_BANK_1;
	eraseInit.Page = 128;
	eraseInit.NbPages = 1;
	uint32_t eraseError;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();
	HAL_FLASHEx_Erase(&eraseInit, &eraseError);
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}


	// check that flash was erased
	char expected[FLASH_PAGE_SIZE];
	char actual[FLASH_PAGE_SIZE];
	memset(expected, 255, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, FLASH_PAGE_SIZE);
	}

	printFlashData(0x08100000, 0x08100010);

	// data to write to flash
	char flashData[16] __attribute__ ((aligned(16)));
	memset(flashData, 9, 16);

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t) 0x08100000, (uint32_t) &flashData);
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}

	// check that flash was programmed correctly (first quad word)
	memcpy(expected, flashData, 16);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, 16);
	}

	printFlashData(0x08100000, 0x08100010);

	// Erase data after reading
	FLASH_EraseInitTypeDef eraseInit2;
	eraseInit2.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit2.Banks = FLASH_BANK_1;
	eraseInit2.Page = 128;
	eraseInit2.NbPages = 1;
	uint32_t eraseError2;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();
	HAL_FLASHEx_Erase(&eraseInit2, &eraseError2);
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}

	//flashTest_Erase(128);
	//eraseFlashPage(128);


	// check that flash was erased
	memset(expected, 255, FLASH_PAGE_SIZE);
	readFlashPage(128, actual);
	// if testStatus is already HAL_ERROR, then don't change it
	if (testStatus == HAL_OK) {
		testStatus = compareBuffers(expected, actual, FLASH_PAGE_SIZE);
	}

	printFlashData(0x08100000, 0x08100010);


	if (testStatus == HAL_OK) {
		printf("\n\n***************************************\nFLASH TEST 3 - Test Passed\n***************************************\n\n");
	} else if (testStatus == HAL_ERROR) {
		printf("\n\n***************************************\nFLASH TEST 3 - Test Failed\n***************************************\n\n");
	}
}

// Literately copy pasted from flashTest_1() and renamed to flashTest_4(), that's it.
// Sanity check...
void flashTest_4() {
	// Test flash simplest case
	printf("\n\n***************************************\nFLASH TEST 1\n***************************************\n\n");

	// Erase page 128
	FLASH_EraseInitTypeDef eraseInit;
	eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit.Banks = FLASH_BANK_1;
	eraseInit.Page = 128;
	eraseInit.NbPages = 1;
	uint32_t eraseError;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();

	// TODO: changed to not check status
	HAL_FLASHEx_Erase(&eraseInit, &eraseError);
//	if (HAL_FLASHEx_Erase(&eraseInit, &eraseError) != HAL_OK) {
//		printf("Failed to erase flash. eraseError = %ld\n", eraseError);
//	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}

	printFlashData(0x08100000, 0x08100010);

	// data to write to flash
	char flashData[16] __attribute__ ((aligned(16)));
	memset(flashData, 9, 16);

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();

	// TODO: changed to not check status
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t) 0x08100000, (uint32_t) &flashData);
//	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t) 0x08100000, (uint32_t) &flashData)
//			!= HAL_OK) {
//		printf("Failed to write to flash.\n");
//	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}

	// read from flash
	//	__IO uint32_t *readAddress = (uint32_t *) 0x08100000;
	//	uint32_t readData = *readAddress;
	//	printf("Reading from flash...  ");
	//	printf("Read: %ld\n", readData);
	//
	//	// test read from flash
	//	for (int i = 0; i < 4; i++) {
	//		printf("%08X\n", (unsigned int) readFlash(0x08100000, i));
	//	}

	printFlashData(0x08100000, 0x08100010);

	// Erase data after reading
	FLASH_EraseInitTypeDef eraseInit2;
	eraseInit2.TypeErase = FLASH_TYPEERASE_PAGES;
	eraseInit2.Banks = FLASH_BANK_1;
	eraseInit2.Page = 128;
	eraseInit2.NbPages = 1;
	uint32_t eraseError2;

	if (HAL_FLASH_Unlock() != HAL_OK) {
		printf("Failed to unlock flash\n");
	}
	clearAllFlashFlags();

	// TODO: Changed to not check status
	HAL_FLASHEx_Erase(&eraseInit2, &eraseError2);
//	if (HAL_FLASHEx_Erase(&eraseInit2, &eraseError2) != HAL_OK) {
//		printf("Failed to erase flash. eraseError = %ld\n", eraseError2);
//	}
	if (FLASH_WaitForLastOperation(HAL_MAX_DELAY) != HAL_OK) {
		printf("Wait failed...\n");
	}
	if (HAL_FLASH_Lock() != HAL_OK) {
		printf("Failed to lock flash\n");
	}
	//flashTest_Erase(128);
	//eraseFlashPage(128);

	printFlashData(0x08100000, 0x08100010);

//	if (testStatus == HAL_OK) {
//		printf("\n\n***************************************\nFLASH TEST 5 - Test Passed\n***************************************\n\n");
//	} else if (testStatus == HAL_ERROR) {
//		printf("\n\n***************************************\nFLASH TEST 5 - Test Failed\n***************************************\n\n");
//	}
}

/**
 * Test write page.
 */
void flashTest_5() {
	HAL_StatusTypeDef testStatus = HAL_OK;
	printf("\n\n***************************************\nFLASH TEST 5 - Flash Page Write\n***************************************\n\n");
	char writeBuffer[FLASH_PAGE_SIZE];
	memset(writeBuffer, 2, FLASH_PAGE_SIZE);
	eraseFlashPage(128);
	writeFlashPage(128, writeBuffer);
	printFlashData(0x08100000, 0x08100010);

	// check if data was written correctly
	char readBuffer[FLASH_PAGE_SIZE];
	readFlashPage(128, readBuffer);

	// compare read buffer and write buffer
	testStatus = compareBuffers(writeBuffer, readBuffer, FLASH_PAGE_SIZE);

	if (testStatus == HAL_OK) {
		printf("\n\n***************************************\nFLASH TEST 5 - Test Passed\n***************************************\n\n");
	} else if (testStatus == HAL_ERROR) {
		printf("\n\n***************************************\nFLASH TEST 5 - Test Failed\n***************************************\n\n");
	}
}

/**
 * Compares two buffer to see if contents are the same. Buffers should be the same length.
 * @param   buffer1
 * @param   buffer2
 * @param   bufferLength
 * @retval  HAL_OK if buffers are the same, HAL_ERROR if buffers are different.
 */
HAL_StatusTypeDef compareBuffers(char *buffer1, char *buffer2, int bufferLength) {
	HAL_StatusTypeDef sameStatus = HAL_OK;
	for (int i = 0; i < bufferLength; i++) {
		// if any read data does not match write data, set testStatus to HAL_ERROR
		if (buffer1[i] != buffer2[i]) {
			sameStatus = HAL_ERROR;
		}
	}
	return sameStatus;
}
