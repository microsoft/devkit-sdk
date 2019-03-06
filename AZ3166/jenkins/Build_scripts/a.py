import os
import fnmatch

dirpath = os.getcwd()
foldername = os.path.basename(dirpath)

output = ""

for root, dirnames, filenames in os.walk('.'):
    for filename in fnmatch.filter(filenames, '*.o'):
        output += "\"" + os.path.join(root, filename).replace('\\', '/') + "\" "

outputFileName = foldername + 'Files.txt'
with open(outputFileName, 'w') as files:
    files.write(output)

cmd = "" + "\"C:\\Program Files (x86)\\GNU Tools ARM Embedded\\6.2 2016q4\\bin\\arm-none-eabi-ar\"" + " rcs lib" + foldername + ".a @" + outputFileName
print(cmd)
os.system(cmd)