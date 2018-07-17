#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
using namespace std;

void PrintUsage()
{
	cout << "Usage: CRC16Calculator filepath.\n";
}

uint16_t calcCrc(vector<char> data)
{
	uint16_t crc = 0;

	for (char& c : data)
	{
		crc ^= (short)(c << 8);
		for (int i = 0; i < 8; i++)
		{
			if ((crc & 0x8000) != 0)
				crc = (short)((crc << 1) ^ 0x1021);
			else
				crc = (short)(crc << 1);
		}
	}
	return crc;
}

int main(int argc, char *argv[])
{
	string filePath = "";
	if (argc <= 1)
	{
		PrintUsage();
		return 1;
	}
	filePath = string(argv[1]);
	ifstream fin(filePath, ios::binary | ios::ate);
	if (!fin.good())
	{
		cout << "File Not Exist: " << filePath << "\n";
		PrintUsage();
		return 1;
	}
	ifstream::pos_type pos = fin.tellg();
	vector<char> fileBytes(pos);

	fin.seekg(0, ios::beg);
	fin.read(&fileBytes[0], pos);
	fin.close();

	uint16_t checksum = calcCrc(fileBytes);
	stringstream sstream;
	sstream << std::setfill('0') << setw(4) << std::hex << checksum;
	string hex(sstream.str());
	cout << "File size: " << fileBytes.size() << ", CRC16 Checksum: " << hex << "\n";

	return 0;
}
