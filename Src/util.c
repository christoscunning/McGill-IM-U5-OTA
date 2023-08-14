/**
 ******************************************************************************
 * @file           util.c
 * @brief          General utility functions
 ******************************************************************************
 */

/* Includes -----------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "stm32u5xx_hal.h"

/* Private variables --------------------------------------------------------*/

/* Functions ----------------------------------------------------------------*/

/**
 * Print a buffer of a specified length using the specified formatting.
 *
 * @param buffer Char buffer to be printed
 * @param length Amount of characters to be printed from buffer
 * @param format The format specifier used to print the elements of the char array (ex: "%c" or "%x")
 */
void printBuffer(char *buffer, int length, char *format) {
	char buffer_name[5] = "buff\0";
	printf("%s: ", buffer_name);
	for (int i = 0; i < length; i++) {
		printf(format, buffer[i]);
	}
	printf("\n");
}

/**
 * Returns 0 if little Endian, 1 if big Endian.
 */
int isBigEndian() {
	const int i = 1;
	return ((*(char *) &i) == 0);
}

/**
 * Use to fix endianness of buffer. Buffer length must be multiple of 4.
 *
 * @param buffer
 * @param size Size of buffer in bytes.
 */
HAL_StatusTypeDef fixEndianness(uint8_t *buffer, int size) {
	if (size % 4 != 0) {
		return HAL_ERROR;
	}
	for (int i = 0; i < size / 4; i++) {
		//uint8_t *temp = (uint8_t *) (buffer + (i*4));
		unsigned char c0, c1, c2, c3;
		c0 = buffer[i*4 + 0];
		c1 = buffer[i*4 + 1];
		c2 = buffer[i*4 + 2];
		c3 = buffer[i*4 + 3];
		if (isBigEndian()) {
			buffer[i*4 + 0] = c0;
			buffer[i*4 + 1] = c1;
			buffer[i*4 + 2] = c2;
			buffer[i*4 + 3] = c3;
		} else {
			buffer[i*4 + 0] = c3;
			buffer[i*4 + 1] = c2;
			buffer[i*4 + 2] = c1;
			buffer[i*4 + 3] = c0;
		}
	}

	return HAL_OK;
}

/**
 * @brief Computes the SHA256 hash value of an array of bytes in flash memory.
 *
 * @param hhash The HASH handle.
 * @param flashAddress The starting address of the byte array in memory.
 * @param size The length of the byte array to compute the hash for.
 * @param digest Pointer to the 32 byte array that will store the SHA256 output digest.
 * @retval Status of the operation.
 */
HAL_StatusTypeDef computeHashFromFlash(HASH_HandleTypeDef *hhash, uint32_t flashAddress, int size, char *digest) {
	// Compute hash 1 flash page at a time.
	int numPages = size / FLASH_PAGE_SIZE;
	int leftOverBytes = size - (numPages * FLASH_PAGE_SIZE);
	printf("Computing SHA256 Hash of %d bytes in flash starting at address: %08lx\n", size, flashAddress);

	char firmwarePage[FLASH_PAGE_SIZE];

	for (int i = 0; i < numPages; i++) {
		memcpy(firmwarePage, (char *) flashAddress, FLASH_PAGE_SIZE);
		fixEndianness((uint8_t *) firmwarePage, FLASH_PAGE_SIZE);
		if (HAL_HASHEx_SHA256_Accmlt(hhash, (uint8_t *) firmwarePage, FLASH_PAGE_SIZE) != HAL_OK) {
			printf("Error: accumulating hash at addr = %08lx\n", flashAddress);
			return HAL_ERROR;
		}
		flashAddress += FLASH_PAGE_SIZE;
	}

	// do leftover and call accumlt_end
	memcpy(firmwarePage, (char *) flashAddress, leftOverBytes);
	fixEndianness((uint8_t *) firmwarePage, leftOverBytes);
	if (HAL_HASHEx_SHA256_Accmlt_End(hhash, (uint8_t *) firmwarePage, leftOverBytes, (uint8_t *) digest, HAL_MAX_DELAY) != HAL_OK) {
		printf("Error: calling accmlt_end\n");
		return HAL_ERROR;
	}

	return HAL_OK;
}




// TESTING

void testEndianness(HASH_HandleTypeDef *hhash) {
	uint8_t bufferForward[] = {0xfc, 0x7f, 0x00, 0x20};
	uint8_t bufferReverse[] = {0x20, 0x00, 0x7f, 0xfc};
	fixEndianness(bufferForward, 4);
	fixEndianness(bufferReverse, 4);
	uint32_t forwardInt = (*(__IO uint32_t*) bufferForward);
	uint32_t reverseInt = (*(__IO uint32_t*) bufferReverse);
	printf("Forward Int: %08lx\n", forwardInt);
	printf("Reverse Int: %08lx\n", reverseInt);


	// check sha256 hash of firmware data
	char forwardHash[32];
	HAL_HASHEx_SHA256_Start(hhash, bufferForward, 4, (uint8_t*) forwardHash, HAL_MAX_DELAY);
	printf("Forward sha256 hash: \n");
	printBuffer(forwardHash, 32, "%02x");
	printf("\n");

	char reverseHash[32];
	HAL_HASHEx_SHA256_Start(hhash, bufferReverse, 4, (uint8_t*) reverseHash, HAL_MAX_DELAY);
	printf("Reverse sha256 hash: \n");
	printBuffer(reverseHash, 32, "%02x");
	printf("\n");
}
