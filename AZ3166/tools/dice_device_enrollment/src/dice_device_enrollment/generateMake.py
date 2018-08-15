import fnmatch
import os

def generateMakefile(rootdir = '.', outputName = 'dps_cert_gen_mac'):
	sourceFiles = []
	includePaths = set()
	for looproot, _, filenames in os.walk(rootdir):
		for filename in filenames:
			if fnmatch.fnmatch(filename, '*.c*'):
				sourceFiles.append([filename[:filename.index('.')] + '.o', looproot, filename])
			elif fnmatch.fnmatch(filename, '*.h'):
				if (looproot != '.'):
					includePaths.add(looproot[2:])
	targets = ""
	for a in sourceFiles: targets += " " + a[0]
	print outputName + ":" + targets

	includeFlag = ""
	for a in includePaths: includeFlag += " -I" + a
	print("CXXFLAGS=-Os -lstdc++ " + includeFlag)

	for a in sourceFiles:
		if (a[1] != "."):
			print(a[0] + ":\t" + os.path.join(a[1], a[2]))

	for a in sourceFiles:
		if a[2].endswith('.cpp'):
			print("\tc++ $(CXXFLAGS) -c -o " + a[0] + " " + os.path.join(a[1], a[2]))
		else :
			print("\tgcc $(CXXFLAGS) -c -o " + a[0] + " " + os.path.join(a[1], a[2]))

	print("\tc++ -o " + outputName + targets)
	print("\trm *.o")

	print("\nclean:")
	print("\trm -f *.o")
	print("\trm -f " + outputName)

generateMakefile()

