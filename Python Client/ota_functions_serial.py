import serial
from time import sleep


class RFCOMM_Connection:
    #address = None
    #services = None
    
    def __init__(self, address):
        self.ser = None
        
    def add_service(self, com_port, verbose=False):
        if self.ser is not None and self.ser.is_open:
            print("Port already open...")
            return False
        
        self.ser = serial.Serial(port=com_port, baudrate=115200, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, rtscts=False)
        
        return True
        
    def get_service(self, service_uuid):
        if service_uuid not in self.services:
            print("Error service does not exist.")
            return False
        else:
            return self.services[service_uuid]
        
    def close_service(self):
        self.ser.close()
        
    def send(self, data):
        #self.services[service_uuid].send(data)
        self.ser.write(data)
        
    def recv(self, recv_len):
        try:
            #data = self.services[service_uuid].recv(recv_len)
            data = self.ser.read(recv_len)
            #print("Return data: ", ' '.join('{:02x}'.format(x) for x in data))
            return data
        except OSError:
            print("OSError")
            return None
        

       

""" # General Utility Functions # """

def load_firmware_from_file(filepath):
    """
    Loads firmware data from a file to a binary data object.
    """
    file = open(filepath, "rb")
    firmware_data = file.read()
    # close file
    file.close()
    return firmware_data
    
def get_size_of_firmware(filepath):
    """
    Returns the size of the firmware file, in number of bytes.
    """
    file = open(filepath, "rb")
    file.seek(0,2)
    firmware_size = file.tell()
    file.seek(0,0)
    file.close()
    return firmware_size    

def print_long_hex(hex_data, line_len=16, print_len=-1):
    """
    Print out a binary data object (bytes or bytearray) formatted as hexidecimal data.
    line_len can be used to configure the number of hex characters printed per line.
    print_len can be used to specify how many lines of the binary data should be
    printed. If print_len is not specified, then all the binary data will be 
    printed.
    """
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
    """ 
    Returns a string representation of the bytes input that is readable
    when printed
    """
    if b == None:
        return ''
    return ' '.join('0x{:02X}'.format(x) for x in b)