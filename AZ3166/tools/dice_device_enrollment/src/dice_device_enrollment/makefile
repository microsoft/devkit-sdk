dps_cert_gen_mac: DiceRIoT.o dice_device_enrollment.o RiotCore.o RiotAesTables.o RiotSha256.o RiotEcc.o RiotDerEnc.o def.o RiotAes128.o RiotX509Bldr.o RiotBase64.o RiotHmac.o RiotCrypt.o RiotKdf.o DiceCore.o DiceSha256.o
CXXFLAGS=-Os -lstdc++  -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include
RiotCore.o:	./RIoT/Core/RiotCore.c
RiotAesTables.o:	./RIoT/Core/RIoTCrypt/RiotAesTables.c
RiotSha256.o:	./RIoT/Core/RIoTCrypt/RiotSha256.c
RiotEcc.o:	./RIoT/Core/RIoTCrypt/RiotEcc.c
RiotDerEnc.o:	./RIoT/Core/RIoTCrypt/RiotDerEnc.c
def.o:	./RIoT/Core/RIoTCrypt/def.c
RiotAes128.o:	./RIoT/Core/RIoTCrypt/RiotAes128.c
RiotX509Bldr.o:	./RIoT/Core/RIoTCrypt/RiotX509Bldr.c
RiotBase64.o:	./RIoT/Core/RIoTCrypt/RiotBase64.c
RiotHmac.o:	./RIoT/Core/RIoTCrypt/RiotHmac.c
RiotCrypt.o:	./RIoT/Core/RIoTCrypt/RiotCrypt.c
RiotKdf.o:	./RIoT/Core/RIoTCrypt/RiotKdf.c
DiceCore.o:	./DICE/DiceCore.c
DiceSha256.o:	./DICE/DiceSha256.c
	c++ $(CXXFLAGS) -c -o DiceRIoT.o ./DiceRIoT.cpp
	c++ $(CXXFLAGS) -c -o dice_device_enrollment.o ./dice_device_enrollment.cpp
	gcc $(CXXFLAGS) -c -o RiotCore.o ./RIoT/Core/RiotCore.c
	gcc $(CXXFLAGS) -c -o RiotAesTables.o ./RIoT/Core/RIoTCrypt/RiotAesTables.c
	gcc $(CXXFLAGS) -c -o RiotSha256.o ./RIoT/Core/RIoTCrypt/RiotSha256.c
	gcc $(CXXFLAGS) -c -o RiotEcc.o ./RIoT/Core/RIoTCrypt/RiotEcc.c
	gcc $(CXXFLAGS) -c -o RiotDerEnc.o ./RIoT/Core/RIoTCrypt/RiotDerEnc.c
	gcc $(CXXFLAGS) -c -o def.o ./RIoT/Core/RIoTCrypt/def.c
	gcc $(CXXFLAGS) -c -o RiotAes128.o ./RIoT/Core/RIoTCrypt/RiotAes128.c
	gcc $(CXXFLAGS) -c -o RiotX509Bldr.o ./RIoT/Core/RIoTCrypt/RiotX509Bldr.c
	gcc $(CXXFLAGS) -c -o RiotBase64.o ./RIoT/Core/RIoTCrypt/RiotBase64.c
	gcc $(CXXFLAGS) -c -o RiotHmac.o ./RIoT/Core/RIoTCrypt/RiotHmac.c
	gcc $(CXXFLAGS) -c -o RiotCrypt.o ./RIoT/Core/RIoTCrypt/RiotCrypt.c
	gcc $(CXXFLAGS) -c -o RiotKdf.o ./RIoT/Core/RIoTCrypt/RiotKdf.c
	gcc $(CXXFLAGS) -c -o DiceCore.o ./DICE/DiceCore.c
	gcc $(CXXFLAGS) -c -o DiceSha256.o ./DICE/DiceSha256.c
	c++ -o dps_cert_gen_mac DiceRIoT.o dice_device_enrollment.o RiotCore.o RiotAesTables.o RiotSha256.o RiotEcc.o RiotDerEnc.o def.o RiotAes128.o RiotX509Bldr.o RiotBase64.o RiotHmac.o RiotCrypt.o RiotKdf.o DiceCore.o DiceSha256.o
	rm *.o

clean:
	rm -f *.o
	rm -f dps_cert_gen_mac
