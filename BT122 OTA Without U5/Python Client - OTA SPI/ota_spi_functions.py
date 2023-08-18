import bluetooth
from time import sleep


class RFCOMM_Connection:
    #address = None
    #services = None
    
    def __init__(self, address):
        self.address = address
        self.services = dict()
        
    def add_service(self, service_uuid, verbose=False):
        # check if service already exists
        if service_uuid in self.services:
            print("Error: Service already exists.")
            return False
        
        # search for service matching uuid
        service_matches = bluetooth.find_service(uuid=service_uuid, address=self.address)

        # Check for service match
        if len(service_matches) == 0:
            print("Error: Could not find service with uuid: '" + str(service_uuid) + "'")
            return False
            
        first_match = service_matches[0]
        port = first_match["port"]
        name = first_match["name"]
        host = first_match["host"]
        
        if verbose:
            print("Connecting to \'{}\' on {}...".format(name.decode('utf-8'), host))

        # Create client RFCOMM socket
        server_socket = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        server_socket.connect((host, port))
        if verbose:
            print("Connected to '" + name.decode('utf-8') + "' service.")
        
        self.services[service_uuid] = server_socket
        
        return True
        
    def get_service(self, service_uuid):
        if service_uuid not in self.services:
            print("Error service does not exist.")
            return False
        else:
            return self.services[service_uuid]
        
    def close_service(self, service_uuid):
        self.services[service_uuid].close()
        
    def send(self, service_uuid, data):
        self.services[service_uuid].send(data)
        
    def recv(self, service_uuid, recv_len):
        try:
            data = self.services[service_uuid].recv(recv_len)
            #print("Return data: ", ' '.join('{:02x}'.format(x) for x in data))
            return data
        except OSError:
            print("OSError")
            return None
        
        

class SPI_Connection:
    socket = None
    control_map = None    
    
    def __init__(self, socket, control_map):
        """ Create a new SPI connection object
        socket = Bluetooth socket communicating with BT122
        control_map = map of control characters to SPI operation
        """
        self.socket = socket
        self.control_map = control_map

    """ FLASH FUNCTIONS """

    # helper function
    def send_control_char(self, control_char_int, control_data, return_data_len=0, control_desc="", verbose=True):
        control_char = (control_char_int).to_bytes(1, byteorder='little')
        send_buffer = bytearray(control_char)
        if control_data is not None:
            data_bytes = bytes.fromhex(control_data)
            for data in data_bytes:
                send_buffer.append(data)
        if verbose:
            print("Sending control character '{}' -> {}".format(control_char, control_desc))
        self.socket.send(bytes(send_buffer))
        # wait to receive return data
        data = None
        if return_data_len != 0:
            try:
                data = self.socket.recv(return_data_len)
                if verbose:
                    #print("Return data: ", ' '.join('{:02x}'.format(x) for x in data))
                    pass
            except OSError:
                print("OSError")
                pass
        # wait to receive response
        try:
            confirmation = self.socket.recv(1)
            if verbose:
                #print("Received: ", confirmation)
                pass
        except OSError:
            print("OSError")
            pass
        # return data if present
        if data is not None:
            return data
           
           
    def echo(self, data_len, data, verbose=False):
        """
        Test with echo
        """
        control_char = self.control_map["ECHO"]
        response = self.send_control_char(control_char, data, return_data_len=data_len, control_desc="BT122 Echo test", verbose=verbose)   
        if response == None and verbose:
            print("Error: no response received.")
        if response != None and verbose:
            print("\t Echo received: " + ' '.join(hex(x) for x in response))
        return response


    # Send 'turn on' to external FLASH by writing 0x04 to control characteristic
    def turn_on_flash(self):
        control_char = (4).to_bytes(1, byteorder='little')
        print("Sending control characteristic: '{}' -> turn on external flash".format(control_char))
        self.socket.send(control_char)

        # wait to receive response '0x00'
        try:
            data = self.socket.recv(1)
            #print("Received: ", data)
        except OSError:
            print("OSError")
            pass
            
        # pause to wait for flash to start
        sleep(2)
        
    def flash_read_status_register(self):
        control_char = self.control_map["RDSR"]
        data = self.send_control_char(control_char, None, return_data_len=1, control_desc="read status register")
        print("\tStatus register: ", data)
        
    def flash_write_enable(self):
        self.send_control_char(8, None, return_data_len=0, control_desc="Write enable (WREN)")

    # Erase flash block
    def flash_block_erase(self):
        control_char = self.control_map["BKER"]
        control_byte = (control_char).to_bytes(1, byteorder='little')
        print("Sending control character '{}' -> erase flash".format(control_byte))
        self.socket.send(control_byte)
        # wait to receive result codes
        for i in range(4):
            try:
                data = self.socket.recv(2)
                print("\tErase op result code {}: ".format(i), ' '.join('{:02x}'.format(x) for x in data))
            except OSError:
                print("OSError")
                pass
            
        # wait to receive response
        try:
            data = self.socket.recv(1)
            print("Received: ", data)
        except OSError:
            print("OSError")
            pass
            
    # CHIP erase flash
    def flash_chip_erase(self):
        control_char = self.control_map["CHER"]
        control_byte = (control_char).to_bytes(1, byteorder='little')
        print("Sending control char '{}' -> chip erase flash".format(control_byte))
        self.socket.send(control_byte)
        try:
            data = self.socket.recv(2)
            print("\tChip erase op result code: ", ' '.join('{:02x}'.format(x) for x in data))
        except OSError:
            print("OSError")
            pass
        # wait to receive response
        try:
            data = self.socket.recv(1)
            #print("Received: ", data)
        except OSError:
            print("OSError")
            pass
        

           
    # addr = hex format ex: "abcdef" = 0xabcdef
    def flash_read(self, addr, read_len, verbose=True):
        control_char = self.control_map["READ"]
        control_data = '{:0>2x}'.format(read_len) + addr
        #print("Control data in hex string: ", control_data)
        desc = "Read flash @ addr = 0x" + addr
        # add 3 to read_len for extra return data (len + result_code)
        response = self.send_control_char(control_char, control_data, return_data_len=read_len+3, control_desc=desc, verbose=verbose)
        if read_len != response[0]:
            print("Error did not read requested amount. Expected: {} Actual: {}".format(read_len, response[0]))
            exit(1)
        read_data = response[1:read_len+1]
        result_code = response[read_len+1:read_len+3]
        
        """ discard first 4 bytes of read data """
        #read_data = read_data[4:]
        
        if verbose:
            print("\tRead '" + ' '.join(hex(x) for x in read_data) + "' from addr '0x" + addr + "' with result_code = " + ' '.join(hex(x) for x in result_code))
        return read_data

    # Page write
    # addr = hex format (24 bits / 3 bytes)
    # can only write 1 byte at a time for now
    def flash_write(self, addr, write_len, write_data, verbose=True):
        control_char = self.control_map["PGWR"]
        desc = "Write '0x" + write_data + "' at addr = '0x" + addr + "'"    
        control_data = addr + '{:0>2x}'.format(write_len) + write_data
        response = self.send_control_char(control_char, control_data, return_data_len=2, control_desc=desc, verbose=verbose)
        # print result code
        result_code = response[1:3]
        if verbose:
            print("\tWrote with result code = " + ' '.join(hex(x) for x in result_code))
            
    def dfu_reset(self, verbose = False):
        """ Send command to reset device into DFU boot mode. """
        control_char = self.control_map["DFUR"]
        control_byte = (control_char).to_bytes(1, byteorder='little')
        if verbose:
            print("Sending control char '{}' -> system reset into DFU boot mode".format(control_byte))
        self.socket.send(control_byte)
        # nothing to receive, as device will be rebooting

 
    """ END FLASH FUNCTIONS """


class OTA_Update:
    firmware_filepath = None
    firmware_data = None
    firmware_size = None
    
    def __init__(self):
        self.firmware_filepath = ""
        self.firmware_data = None
        self.firmware_size = -1
        
    def load_firmware_from_file(self, filepath):
        file = open(filepath, "rb")
        self.firmware_filepath = filepath
        # seek to end of file to get the size
        file.seek(0,2)
        self.firmware_size = file.tell()
        # go back to beginning of file and read contents
        file.seek(0,0)
        self.firmware_data = file.read()
        # close file
        file.close()
        return self.firmware_data
        
    def perform_dfu(self, spi, verbose=True):
        """
        Performs the Over-The-Air Device-Firmware-Upgrade.
        Requires first loading a firmware file using 'load_firmware_from_file'
        as well as a valid SPI connection object connected to the 'OTA Control'
        channel of the BT122 device.
        """
        if verbose:
            print("Erasing flash...")
            
        # erase flash
        spi.flash_chip_erase
        if verbose:
            print("Flash erased.")
            print("Writing firmware data to flash...")
            
        # write firmware data to flash
        for i in range(int(self.firmware_size / 32)):
            addr = '{:0>6x}'.format(i * 32)
            data = ''.join('{:0>2x}'.format(x) for x in self.firmware_data[i*32:i*32+32])
            print("Writing data: " + data + "  to addr: " + addr)
            spi.flash_write(addr, 32, data, verbose=False)
            if verbose and i % 500 == 0:
                print("Written: " + str(i*32) + " out of " + str(self.firmware_size) + " bytes.")
        if verbose:
            print("Finished writing to flash.")
            print("Reading flash data...")
        
        # Read flash data (all 256 KB)
        total_data = bytearray()
        for i in range(1000 * 4):
            addr = '{:0>6x}'.format(i * 64)
            #print("reading from addr: " + addr)
            total_data += bytearray(spi.flash_read(addr, 64, verbose=False))
            if verbose and i % 800 == 0:
                print("Read: " + str(i * 64) + " out of " + str(4000 * 64) + " bytes.")
        if verbose:
            print("Finished reading flash.")
            print("Verifying firmware data written correctly...")
        
        # verify firmware data written correctly
        valid = compare_bin_contents(total_data, self.firmware_data)
        if valid:
            if verbose:
                print("Firmware data valid.")
                print("Resetting device into DFU mode...")
            # send dfu reset control code
            spi.dfu_reset()
            print("OTA DFU Complete.")
        else:
            print("Firmware data not valid.")
        
        
        

""" # General Utility Functions # """

def print_long_hex(hex_data, line_len=16, print_len=-1):
    # formatting
    print("")
    line = "Address |"
    for i in range(line_len):
        line += "{}".format('{:x}'.format(i)).rjust(2, " ") + " "
    print(line)
    line = "--------+"
    for i in range(line_len):
        line += "---"
    print(line)
    
    # print data    
    lines = len(hex_data) / line_len
    # if print_len specified, do not print all data
    if print_len != -1:
        lines = print_len / line_len
    addr = 0
    #print("Printing " + str(lines) + " lines of hex")
    for i in range(int(lines)):
        print('{:0>8x}'.format(addr) + "|" + ' '.join('{:0>2x}'.format(x) for x in hex_data[i*line_len:i*line_len+line_len]))
        addr += line_len
    print("")


def compare_bin_contents(data1, data2):
    """
    Returns True if files are identical, False otherwise
    """
    # compare
    same = True
    for i in range(256):
        if data1[i] != data2[i]:
            print("Found mismatch at i = " + str(i))
            same = False
            break


    if same:
        print("Files are identical")
    else:
        print("Files are different")
        
        
    return same
        

def print_bytes(b):
    """ Returns a string representation of the bytes input that is readable
    when printed
    """
    if b == None:
        return ''
    return ' '.join('0x{:02X}'.format(x) for x in b)