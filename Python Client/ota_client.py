#!/usr/bin/env python3

import sys
from hashlib import md5, sha256
from ota_functions import RFCOMM_Connection, load_firmware_from_file, get_size_of_firmware, print_long_hex, print_bytes

bt122_MAC_addr = 'c4:64:e3:64:0a:5a'
uuid_spp = "1101"

# Create rfcomm connection
rf = RFCOMM_Connection(bt122_MAC_addr)
if rf.add_service(uuid_spp, verbose=True) == False:
    print("Failed to add service")
    exit(1)


# Connection confirmation
confirmation = bytearray([1,2])
rf.send(uuid_spp, bytes(confirmation))
print("Sent confirmation: " + str(confirmation))
confirmation = rf.recv(uuid_spp, 2)
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
rf.send(uuid_spp, firmware_size_bytes)


# apply sha256 to firmware data
firmware_hash = sha256()
firmware_hash.update(firmware_data)
firmware_digest = firmware_hash.hexdigest()
print("Firmware SHA256 digest: " + str(firmware_digest))

# send expected sha256 hash of firmware data
rf.send(uuid_spp, firmware_hash.digest())


# wait for user input to confirm transmission of firmware image (optional)
input("press enter to send firmware")

# send entire firmware file
# send 8192 bytes at a time. 
# If you change this value, also make sure to change CHUNK_SIZE variable in the ota.c downloadFirmwareToFlash() function
chunkSize = 8192 
# keep track of how many bytes have been sent
bytes_written = 0 
# Number of chunks that will be sent
numChunks = int(firmware_size // chunkSize)
# firmware_size = (chunkSize * numChunks) + leftOverBytes
# ex: IF (firmware_size = 80) & (chunkSize = 32) THEN (numChunks = 2) & (leftOverBytes = 16)
leftOverBytes = int(firmware_size - (numChunks * chunkSize)) 
for i in range(0, numChunks):
    firmware_chunk = firmware_data[i*chunkSize:(i+1)*chunkSize]
    rf.send(uuid_spp, firmware_chunk)
    bytes_written += chunkSize
    print("wrote chunk, waiting for confirmation before next chunk")
    reply = rf.recv(uuid_spp, 1)
    print("received reply: " + str(reply) + ", sending next bytes.")


# send left over bytes (firmware data may not be equally divisible by chunkSize)
if leftOverBytes > 0:
    firmware_chunk = firmware_data[numChunks*chunkSize:numChunks*chunkSize+leftOverBytes]
    rf.send(uuid_spp, firmware_chunk)
    bytes_written += leftOverBytes
    print("wrote left over bytes, waiting for confirmation before finishing")
    reply = rf.recv(uuid_spp, 1)
    print("received reply: " + str(reply) + ", finished sending firmware data")


# firmware data transmission complete
print("Firmware upload complete. Sent " + str(bytes_written) + " bytes of firmware")



# close rfcomm service connection
rf.close_service(uuid_spp)
