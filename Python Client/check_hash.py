#!/usr/bin/env python3
import sys
from hashlib import md5, sha256
from ota_functions import RFCOMM_Connection, load_firmware_from_file, get_size_of_firmware, print_long_hex, print_bytes


# check for command line arguments
if len(sys.argv) > 1:
    filepath = sys.argv[1]
else:
    #filepath = "./firmware_files/BT122_UART_STREAMING_2.0.bin"
    filepath = "./firmware_files/U5A5_OTA_DFU_2.0.bin"

# Load firmware file
firmware_data = load_firmware_from_file(filepath)
firmware_size = get_size_of_firmware(filepath)
print("firmware length: " + str(firmware_size))
#print_long_hex(firmware_data, print_len=8192)


print_long_hex(firmware_data[0:16], print_len=16)

# apply sha256 to firmware data
firmware_hash = sha256()
firmware_hash.update(firmware_data)
firmware_digest = firmware_hash.hexdigest()

# print sha256 digest
print("Firmware SHA256 digest: " + str(firmware_digest))