#!/usr/bin/env python3
"""
Testing spi interface on BT122

Note on pybluez installation: https://github.com/pybluez/pybluez/issues/180
"""

import codecs
import bluetooth
from time import sleep
from sys import getsizeof

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
filepath = "./firmware_files/BT122_OTA_SPI_2.0.ota"
ota.load_firmware_from_file(filepath)

input("Press any key to begin SPI test...")





        
# test with echo
#data = send_control_char(0, "01abcdef", return_data_len=4, control_desc="test joint control + data")
data = spi.echo(4, "01abcdef", verbose=True)   
print("\t Received: " + ' '.join(hex(x) for x in data))

# toggle LED
spi.send_control_char(4, "01", control_desc="turn on LED")
sleep(1)

# start with CS HIGH
spi.send_control_char(5, "01", control_desc="turn off LED")
sleep(1)
    
    
""" Begin Flash testing """


#spi.turn_on_flash() # unneeded...

#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_read_status_register()
#spi.send_control_char(5, "02", control_desc="toggle CS")


#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_write_enable()
#spi.send_control_char(5, "02", control_desc="toggle CS")


#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_read_status_register()
#spi.send_control_char(5, "02", control_desc="toggle CS")

#call flash erase
#flash_erase()

#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_chip_erase()
#spi.send_control_char(5, "02", control_desc="toggle CS")


#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_read_status_register()
#spi.send_control_char(5, "02", control_desc="toggle CS")


#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_write_enable()
#spi.send_control_char(5, "02", control_desc="toggle CS")


#Read / write data to SPI

#spi.send_control_char(5, "02", control_desc="toggle CS")
spi.flash_write("000000", 4, "02020202")
#spi.send_control_char(5, "02", control_desc="toggle CS")

spi.flash_write("00000c", 4, "11335577")
spi.flash_write("00000a", 4, "22446688")
spi.flash_write("000010", 4, "00cc00cc")
spi.flash_write("000020", 4, "11aa11aa")
spi.flash_write("00002a", 2, "dddd")
spi.flash_write("0001fd", 4, "edededed")

# write x bytes

"""
for i in range(16):
    # write 4 bytes at a time
    addr = '{:0>6x}'.format(i * 32)
    data = '{:0>64x}'.format(77194726158210796949047323339125271902179989777093709359638389338608753093290)
    spi.flash_write(addr, 32, data, verbose=True)
"""
    
# read x bytes

total_data = bytearray()
#total_data += bytearray(flash_read("000000", 64, verbose=True))
for i in range(4):
    addr = '{:0>6x}'.format(i * 64)
    #print("reading from addr: " + addr)
    total_data += bytearray(spi.flash_read(addr, 64, verbose=False))


print_long_hex(total_data)



# Write and Read x Bytes test
"""
# write x bytes

for i in range(16):
    addr = '{:0>6x}'.format(i * 32)
    data = '{:0>64x}'.format(77194726158210796949047323339125271902179989777093709359638389338608753093290)
    spi.flash_write(addr, 32, data, verbose=True)


# read x bytes

total_data = bytearray()
#total_data += bytearray(flash_read("000000", 64, verbose=True))
for i in range(16):
    addr = '{:0>6x}'.format(i * 64)
    #print("reading from addr: " + addr)
    total_data += bytearray(spi.flash_read(addr, 64, verbose=False))

print_long_hex(total_data)
"""

"""

# OTA DFU test

print(type(firmware_data))
print(len(firmware_data))
print('{:x}'.format(firmware_data[0]))
first_four_bytes = ''.join('{:0>2x}'.format(x) for x in firmware_data[0:4])
print(first_four_bytes)


# erase firmware 
flash_chip_erase()

# 40448
print("Writing firmware in "  + str(int(firmware_size/4)) + " iterations.")

# write firmware 32 bytes at a time
for i in range(int(firmware_size / 32)):
    addr = '{:0>6x}'.format(i * 32)
    data = ''.join('{:0>2x}'.format(x) for x in firmware_data[i*32:i*32+32])
    print("Writing data: " + data + "  to addr: " + addr)
    flash_write(addr, 32, data, verbose=False)
    if i % 500 == 0:
        print("Written: " + str(i*32) + " bytes")

# firmware written
print("Finished writing firmware, reading back")


# read all 256 K Bytes of flash
total_data = bytearray()

for i in range(1000 * 4):
    addr = '{:0>6x}'.format(i * 64)
    #print("reading from addr: " + addr)
    total_data += bytearray(flash_read(addr, 64, verbose=True))

#print_long_hex(total_data)
# write data read from flash chip to file
f = open('./flash_contents.bin', 'wb')
f.write(total_data)
f.close()


# send command to reset device into DFU boot mode
control_char = (9).to_bytes(1, byteorder='little')
print("Sending control char '{}' -> system reset into DFU boot mode".format(control_char))
server_socket.send(control_char)

"""


# close connection and exit program
data = input("Press any key to exit...")

# close rfcomm service connection
rf.close_service(uuid_ota_control)