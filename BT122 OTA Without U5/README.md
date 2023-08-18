# BT122 OTA Without U5

Note: I was not quite able to get this to work in the end. The firmware files
seem to be upload correctly to the external flash, but the BT122 does not boot 
into the new firmware. I have included this code even though it does not fully
work, so that maybe somebody can use this and try to figure out why it does 
not work.  


It is possible to upgrade the BT122 firmware by connecting directly to the BT122
device, without using the U5 at all. This is totally separate to the rest of the
code in this GitHub repository, but is included here as an alternative method
of BT122 OTA firmware upgrading. It makes use of an external flash memory connected
to the BT122 device through an SPI interface.  

This method was tested as a possibility, but ultimately was not used, as the BT122
will be connected to the U5 for other reasons anyways, so it made more sense to 
use the flash memory that the U5 already has, rather than use an additional external
flash memory chip.  


It involves two software parts:  
1.  A BGScript Project that implements special functions for OTA  
    -> see README.md in BGScript Project - OTA SPI
2.  A Python client to upload new firmware to the BT122  
    -> see README.md in Python Client - OTA SPI

And two hardware parts:
1.  The BT122 device.
2.  External flash EEPROM, connected through SPI interface to BT122.


Hardware connections: (colors indicate wires that should connect to each other)

Flash:  
Blue   = GND  
Yellow = SCLK  
Brown  = MISO = DQ1  
Red    = MOSI = DQ0  
Orange = CS  
Green  = VDD  

BT122:  
Blue   = GND  
Yellow = SCLK = PC8  
Brown  = MISO = PF4  
Red    = MOSI = PF5  
Orange = CS   = PF2  
Green  = VDD  = PF3  



For more info on flash EEPROM see:  
https://www.st.com/resource/en/datasheet/m95p32-i.pdf  
https://www.st.com/en/ecosystems/x-nucleo-pgeez1.html#documentation  