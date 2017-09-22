import os
import binascii
import struct
import shutil
import inspect
import sys
        		
def binary_hook(binf):
    with open(binf,'rb') as f:
            appbin = f.read()
    with open('./boot.bin', 'rb') as f:
            bootbin = f.read()
    with open(binf ,'wb') as f:
            f.write(bootbin+'\xFF'*(0xc000-len(bootbin))+appbin)
    with open(binf.replace(".bin", ".ota.bin") ,'wb') as f:
            f.write(appbin)
                
if __name__ == '__main__':
    binary_hook(sys.argv[1])