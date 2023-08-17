# Example Python Client for Bluetooth OTA Firmware Upgrade

The example client uses the PyBluez library. See below for steps to install it.

## Installing PyBluez

Note: it is highly recommended to use a Python virtual environment to manage
installed packages for this Python project.

Also Python 3.7 is required to install and use the PyBluez library.

1. Install all PyBluez dependecies required by your system.  
   [GNU/Linux](https://github.com/pybluez/pybluez/blob/master/docs/install.rst#gnulinux-dependencies)  
   [Windows](https://github.com/pybluez/pybluez/blob/master/docs/install.rst#windows-dependencies)  
   [macOS](https://github.com/pybluez/pybluez/blob/master/docs/install.rst#macos-dependencies)
3. Install Python 3.7
4. Open a Terminal / PowerShell / Command Prompt window and navigate to the 
"Python Client" folder.
5. Create a virtual environment using Python 3.7. \
    ```py -3.7 -m venv venv``` \
    Creates a virtual enviroment using Python 3.7 in a folder called venv.
6. Activate the virutal enviroment. (This must be done anytime you want to run 
a Python program using the PyBluez library. \
	```./venv/Scripts/activate```  
7. Install the PyBluez library using the wheel file. \
	```pip install .\PyBluez-0.22-cp37-cp37m-win_amd64.whl```  
8. Run a Python script using the PyBluez library. Remember to run the script 
using python rather than running the script directly. \
	```python uart_client.py```  
7. Deactivate the virtual environment when you are finished. \
	```deactivate```   
9. Done. The PyBluez library is now installed. Remember to reactivate the 
virtual environment anytime you want to run a program using the PyBluez library.


If you are having trouble installing PyBluez, check this link for more info:
https://github.com/pybluez/pybluez/issues/180#issuecomment-408235161


## Running the Python Client
The main Python client program used for testing is the "ota_client.py" program. 
This can be used for both the U5 and BT122 firmware upgrades. Just make sure to 
change the ```filepath``` variable in the "ota_client.py" program to point to 
right firmware file, depending on which device is being upgraded.
