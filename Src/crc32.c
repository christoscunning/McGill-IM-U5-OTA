/**
 * @file crc32.c
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

#include <stdint.h>
#include "crc32.h"

static uint32_t crc_table[256];

/**
 * @brief Reflection is a requirement for the official CRC-32 standard.
 * @param[in] reflect DESC.
 * @param[in] c       DESC.
 * @return DESC.
 * @note You can create CRCs without it, but they won't conform to the standard.
 */
static uint32_t reflect(uint32_t reflect, const unsigned char c)
{
  int i;
  uint32_t val = 0;

  /* Swap bit 0 for bit 7, bit 1 For bit 6, etc.... */
  for (i = 1; i < (c + 1); i++) {
    if (reflect & 1) {
      val |= (1 << (c - i));
    }
    reflect >>= 1;
  }

  return val;
}

void crc32_init()
{
  int i, p;
  /* 0x04C11DB7 is the official polynomial used by PKZip, WinZip and Ethernet. */
  uint32_t polynomial = 0x04C11DB7;

  for (i = 0; i <= 0xFF; i++) {
    crc_table[i] = reflect(i, 8) << 24;

    for (p = 0; p < 8; p++) {
      crc_table[i] = (crc_table[i] << 1) ^ ((crc_table[i] & (1 << 31)) ? polynomial : 0);
    }

    crc_table[i] = reflect(crc_table[i], 32);
  }
}

/**
 * @brief Calculates the CRC32 by looping through each of the bytes in sData.
 * @param[in] crc DESC.
 * @param[in] ptr DESC.
 * @param[in] len DESC.
 * @note For Example usage example, see FileCRC().
 */
static void partial_crc(uint32_t *crc, const unsigned char *ptr, uint32_t len)
{
  while (len--) {
    *crc = ((*crc) >> 8) ^ crc_table[((*crc) & 0xFF) ^ *ptr++];
  }
}

uint32_t crc32_calc(const unsigned char *ptr, uint32_t len)
{
  uint32_t crc = 0xffffffff; /* Initilaize the CRC. */
  partial_crc(&crc, ptr, len);
  return(crc ^ 0xffffffff); /* Finalize the CRC and return. */
}

/*******************************************************************************/
