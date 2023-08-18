# Python Client - OTA SPI

This is the Python client files for the BT122 firmware upgrade using external 
flash and SPI.

Again, PyBluez or PySerial are required. See other Python Client for details
on installing these libraries. PyBluez is easier to use, but harder to install.


ota_spi_functions.py contains classes and functions used to perform the OTA upgrade.

ota_spi_client.py is the script that actually performs the upgrade.

spi_test.py is used for testing the individual spi commands in ota_spi_functions.py

See the comments in each file for info on how to use them.