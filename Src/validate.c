/**
 * @file validate.c
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
 * @deprecated
 * @brief Check that a firmware image is valid. This does not work with BT122 firmware.
 *
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "crc32.h"


//#define FLASH_END 0x1f000
#define FLASH_END 0x3E000

typedef struct {
  uint32_t address;
  uint32_t size;
  uint32_t crc;
} memory_segment;

typedef struct {
  memory_segment sw;
  memory_segment hw;
  memory_segment files;
  uint32_t  segments;
} memory_segments;

static int get_boot_version(uint8_t *data)
{
  //int retval = 0;

  if (data[2] != 0x00) {
      
    printf("Error %d\n", 10);
    return 0;
  }

  if (data[3] != 0x20) {
    printf("Error %d\n", 20);
    return 0;
  }

  for (int i = 4; i < 0xc0; i += 4) {
    if (data[i + 2] != 0x00) {
        
    printf("Error %d\n", 30);
      return 0;
    }

    //if (data[i + 3] != 0x08) {
    // this seems to be for the old 121 firmware format. Bt122 firmware has 0x00 in these positions instead
    if (data[i + 3] != 0x00) {
      
    printf("Error %d\n", 40);
      return 0;
    }
  }

  if (data[0x1000 + 3] == 0xff) {
    /* Bootloader v1. */
    //for (int i = 0; i < 0x0f60; i += 4) {
    // bt122 firmware 0xff only goes to 1750 (0x1000 + 0x0750)
    for (int i = 0; i < 0x0750; i += 4) {
      if (data[0x1000 + i] != 0xff) {
          
        printf("Error %d\n", 50);
        return 0;
      }
    }
    return 1;
  } else {
    return 2;
  }
}

static int validate_segment(uint8_t *data, memory_segment *segment)
{
  if (!segment->size || (segment->size > FLASH_END)) {
    return 0;
  }

  if (segment->address > FLASH_END) {
    return 0;
  }

  uint32_t crc;
  crc = crc32_calc(data + segment->address, segment->size);

  if (crc != segment->crc) {
    return 0;
  }

  return 1;
}

int validate_fw(uint8_t *data, unsigned int size, unsigned int load_address)
{
  memory_segments segments;
  unsigned int offset = 0;

  crc32_init();

  if (size < 0x2000) {
      printf("error: -1\n");
    return 0;
  }

  int boot_version = 0;

  if (load_address) {
    boot_version = 1;
    offset = 0x2000;
  } else {
    boot_version = get_boot_version(data);
    offset = 0x1000;
  }

  if (boot_version == 1) {
    offset = 0x2000;
  } else {
    offset = 0x1000;
  }
  
  // offset for bt122 firmware 
  offset = 0x1800;

  if (boot_version == 0) {
    printf("Error %d\n", 0);
    return 0;
  }

  memcpy(&segments, data + size - (3 * 3 + 1) * 4, (3 * 3 + 1) * 4);

  if (segments.segments < 2) {
    printf("Error %d\n", 1);
    return 0;
  }

  if (segments.segments > 3) {
    printf("Error %d\n", 2);
    return 0;
  }

  if (!validate_segment(data - load_address + offset, &segments.sw)) {
    printf("Error %d\n", 3);
    return 0;
  }

  if (!validate_segment(data - load_address + offset, &segments.hw)) {
    printf("Error %d\n", 4);
    return 0;
  }

  if (segments.segments > 2) {
    if (!validate_segment(data - load_address + offset, &segments.files)) {
      printf("Error %d\n", 5);
      return 0;
    }
  }

  return boot_version;
}

int validate_fw_file(FILE *fp, unsigned int load_address)
{
  long pos0 = ftell(fp);
  int retval = 0;
  uint8_t *buf = malloc(FLASH_END);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);


  printf("File size = %ld\n", size);
  if (size > FLASH_END) {
    goto out;
  }


  fseek(fp, 0, SEEK_SET);
  fread(buf, 1, size, fp);
  retval = validate_fw(buf, size, load_address);

  out:
  fseek(fp, pos0, SEEK_SET);
  free(buf);
  return retval;
}

/*******************************************************************************/
