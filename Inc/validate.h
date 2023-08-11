/**
 * @file validate.h
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
 * @brief Supposed to validate firmware images for BT firmware. However does not work with BT122 firmware.
 */

#ifndef VALIDATE_H
#define VALIDATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

int validate_fw(uint8_t *data, unsigned int size, unsigned int start_address);
int validate_fw_file(FILE *fp, unsigned int start_address);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************/
