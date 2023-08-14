"""
Compare the contents of two binary files.
"""


# Paths of the two files to compare
file1 = "./flash_contents.bin"
file2 = "./firmware/BT122_OTA_DFU.ota"
#file2 = "./firmware/BT122_OTA_DFU.bin"

# load firmware
firmware_file = open(file2, "rb")
firmware_data = firmware_file.read()
firmware_file.close()


# load flash contents
flash_contents = open(file1, "rb")
flash_data = flash_contents.read()
flash_contents.close()


# compare
same = True
for i in range(256):
    if flash_data[i] != firmware_data[i]:
        print("Found mismatch at i = " + str(i))
        same = False
        break


if same:
    print("Files are identical")
else:
    print("Files are different")