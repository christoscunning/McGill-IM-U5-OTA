#!/usr/bin/env python3

import sys
from hashlib import md5, sha256
from ota_functions_serial import RFCOMM_Connection, load_firmware_from_file, get_size_of_firmware, print_long_hex, print_bytes

serial_com_port = "COM9"

# Create rfcomm connection
rf = RFCOMM_Connection("")
if rf.add_service(serial_com_port, verbose=True) == False:
    print("Failed to add service")
    exit(1)


# Connection confirmation
confirmation = bytearray([1,2])
rf.send( bytes(confirmation))
print("Sent confirmation: " + str(confirmation))
confirmation = rf.recv( 2)
print("Received confirmation: " + str(confirmation))


# Load firmware file

# check if any command line arguments supplied
if len(sys.argv) > 1:
    filepath = sys.argv[1]
else:
    #filepath = "./firmware_files/BT122_UART_STREAMING_2.0.bin"
    filepath = "./firmware_files/U5A5_OTA_DFU_2.0.bin"

firmware_data = load_firmware_from_file(filepath)
firmware_size = get_size_of_firmware(filepath)
print("firmware size: " + str(firmware_size))
#print_long_hex(firmware_data, print_len=8192)


# send firmware data size (4 bytes)
firmware_size_bytes = firmware_size.to_bytes(4, "little")
rf.send( firmware_size_bytes)


# apply sha256 to firmware data
firmware_hash = sha256()
firmware_hash.update(firmware_data)
firmware_digest = firmware_hash.hexdigest()
print("Firmware SHA256 digest: " + str(firmware_digest))

# send expected sha256 hash of firmware data
rf.send( firmware_hash.digest())


input("press enter to send firmware")

# send entire firmware file
chunkSize = 8192 # send 8192 bytes at a time
bytes_written = 0 # keep track of how many bytes have been sent
numChunks = int(firmware_size // chunkSize)
leftOverBytes = int(firmware_size - (numChunks * chunkSize)) 
for i in range(0, numChunks):
    firmware_chunk = firmware_data[i*chunkSize:(i+1)*chunkSize]
    rf.send( firmware_chunk)
    bytes_written += chunkSize
    print("wrote chunk, waiting for confirmation before next chunk")
    reply = rf.recv( 1)
    print("received reply: " + str(reply) + ", sending next bytes.")


# send left over bytes (firmware data may not be equally divisible by chunkSize
if leftOverBytes > 0:
    firmware_chunk = firmware_data[numChunks*chunkSize:numChunks*chunkSize+leftOverBytes]
    rf.send( firmware_chunk)
    bytes_written += leftOverBytes
    print("wrote left over bytes, waiting for confirmation before finishing")
    reply = rf.recv( 1)
    print("received reply: " + str(reply) + ", finished sending firmware data")


# firmware data transmission complete
print("Firmware upload complete. Sent " + str(bytes_written) + " bytes of firmware")



# close rfcomm service connection
rf.close_service()
