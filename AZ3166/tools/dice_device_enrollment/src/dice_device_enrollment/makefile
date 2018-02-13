dps_cert_gen_mac: dice_device_enrollment.o DiceRIoT.o DiceCore.o DiceSha256.o RiotCore.o def.o RiotAes128.o RiotAesTables.o RiotBase64.o RiotCrypt.o RiotDerEnc.o RiotEcc.o RiotHmac.o RiotKdf.o RiotSha256.o RiotX509Bldr.o
CXXFLAGS = -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include
DiceCore.o:	./DICE/DiceCore.c
DiceSha256.o:	./DICE/DiceSha256.c
RiotCore.o:	./RIoT/Core/RiotCore.c
def.o:	./RIoT/Core/RIoTCrypt/def.c
RiotAes128.o:	./RIoT/Core/RIoTCrypt/RiotAes128.c
RiotAesTables.o:	./RIoT/Core/RIoTCrypt/RiotAesTables.c
RiotBase64.o:	./RIoT/Core/RIoTCrypt/RiotBase64.c
RiotCrypt.o:	./RIoT/Core/RIoTCrypt/RiotCrypt.c
RiotDerEnc.o:	./RIoT/Core/RIoTCrypt/RiotDerEnc.c
RiotEcc.o:	./RIoT/Core/RIoTCrypt/RiotEcc.c
RiotHmac.o:	./RIoT/Core/RIoTCrypt/RiotHmac.c
RiotKdf.o:	./RIoT/Core/RIoTCrypt/RiotKdf.c
RiotSha256.o:	./RIoT/Core/RIoTCrypt/RiotSha256.c
RiotX509Bldr.o:	./RIoT/Core/RIoTCrypt/RiotX509Bldr.c
	c++ -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o dice_device_enrollment.o ./dice_device_enrollment.cpp
	c++ -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o DiceRIoT.o ./DiceRIoT.cpp
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o DiceCore.o ./DICE/DiceCore.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o DiceSha256.o ./DICE/DiceSha256.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotCore.o ./RIoT/Core/RiotCore.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o def.o ./RIoT/Core/RIoTCrypt/def.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotAes128.o ./RIoT/Core/RIoTCrypt/RiotAes128.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotAesTables.o ./RIoT/Core/RIoTCrypt/RiotAesTables.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotBase64.o ./RIoT/Core/RIoTCrypt/RiotBase64.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotCrypt.o ./RIoT/Core/RIoTCrypt/RiotCrypt.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotDerEnc.o ./RIoT/Core/RIoTCrypt/RiotDerEnc.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotEcc.o ./RIoT/Core/RIoTCrypt/RiotEcc.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotHmac.o ./RIoT/Core/RIoTCrypt/RiotHmac.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotKdf.o ./RIoT/Core/RIoTCrypt/RiotKdf.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotSha256.o ./RIoT/Core/RIoTCrypt/RiotSha256.c
	gcc -g -IRIoT/Core -IDICE -IRIoT -IRIoT/Core/RIoTCrypt/include -c -o RiotX509Bldr.o ./RIoT/Core/RIoTCrypt/RiotX509Bldr.c
	cc -o dps_cert_gen_mac dice_device_enrollment.o DiceRIoT.o DiceCore.o DiceSha256.o RiotCore.o def.o RiotAes128.o RiotAesTables.o RiotBase64.o RiotCrypt.o RiotDerEnc.o RiotEcc.o RiotHmac.o RiotKdf.o RiotSha256.o RiotX509Bldr.o
	rm *.o

clean:
	rm *.o
	rm dps_cert_gen_mac
