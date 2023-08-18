#!/usr/bin/env python3
from ota_spi_functions import OTA_Update, RFCOMM_Connection, SPI_Connection, print_long_hex, print_bytes

#server_addr = 'c4:64:e3:64:2e:d0'
server_addr = 'c4:64:e3:64:0a:5a'
uuid_ota_control = "f7bf3564-fb6d-4e53-88a4-5e37e0326063"
uuid_ota_data = "984227f3-34fc-4045-a5d0-2c581f81a153"
OTA_CONTROL_PORT = 7 # The RFCOMM port number for the control channel. #
OTA_DATA_PORT    = 8 # The RFCOMM port number for the data channel. #


# Create rfcomm connection
rf = RFCOMM_Connection(server_addr)
if rf.add_service(uuid_ota_control, verbose=True) == False:
    exit(1)


# define control map - version for spi_script.bgs BT122 project
control_map = {
    "ECHO": 0,      # echo reply
    "READ": 1,      # read
    "PGWR": 2,     # page write
    "BKER": 3,      # block erase
    "LED8": 4,      # LED 0x0008
    "LED4": 5,      # LED 0x0004
    "CHER": 6,      # chip erase
    "RDSR": 7,      # read status register
    "WREN": 8,      # write enable
    "DFUR": 9       # reset into DFU mode
}

# Create SPI connection
spi = SPI_Connection(rf.get_service(uuid_ota_control), control_map)



# OTA update
ota = OTA_Update()

# location of new firmware file
#filepath = "./firmware_files/BT122_OTA_SPI_2.0.ota"
filepath = "./firmware_files/BT122_OTA_SPI_2.0.bin"

# load file
ota.load_firmware_from_file(filepath)

# perform firmware upgrade
ota.perform_dfu(spi, verbose=True)


# close rfcomm service connection
rf.close_service(uuid_ota_control)