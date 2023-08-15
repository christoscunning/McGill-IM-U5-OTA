# BT122 BGScript Project

This is the BGScript project that defines the firmware used by the BT122 device
during the OTA firmware upgrade.  

It is pretty simple. Essentially all it does it open a RFCOMM connection with
other devices, and echo all data receive on the RFCOMM connection to its UART, 
and vice versa.  

Additionally, there is an interrupt that will change the UART mode of the BT122 
device between DATA and BGAPI mode.