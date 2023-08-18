# BGScript Project - OTA SPI

Provides a variety of functions to perform OTA remotely using SPI and external
flash. The function are accessed by writing control characters to the OTA_CONTROL
channel. See Python client for example.  

Here is the possible control characters:

```
control_map = {\
    "ECHO": 0,      # echo reply\
    "READ": 1,      # read\
    "PGWR": 2,     # page write\
    "BKER": 3,      # block erase\
    "LED8": 4,      # LED 0x0008\
    "LED4": 5,      # LED 0x0004\
    "CHER": 6,      # chip erase\
    "RDSR": 7,      # read status register\
    "WREN": 8,      # write enable\
    "DFUR": 9       # reset into DFU mode\
}\
```