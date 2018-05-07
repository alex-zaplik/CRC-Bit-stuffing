#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <bitset>

#define EXIT_FLAG "01111110"

unsigned int CalcCRC(const unsigned char* aInput, size_t aLength, const unsigned int* apTable)
{
	unsigned int CRC = 0;
	for (size_t i = 0; i<aLength; i++) {
		CRC = apTable[(CRC ^ aInput[i]) & 0xFF] ^ (CRC >> 8);
	}
	return (~CRC) & 0xFFFF;
}

void PrecomputeCRC(unsigned int* apTable)
{
	unsigned int i, j, CRC;

	for (i = 0; i < 256; i++) {
		CRC = i;
		for (j = 0; j < 8; ++j) {
			if (CRC & 0x0001) CRC = (CRC >> 1) ^ 0xA6BC;
			else CRC >>= 1;
		}
		apTable[i] = CRC;
	}
}

std::string stuff_bits(std::string data)
{
	std::stringstream ss;
	char int_counter = 0;

	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (*it == '1') int_counter++;
		else int_counter = 0;
		ss << *it;

		if (int_counter == 5)
		{
			int_counter = 0;
			ss << '0';
		}
	}

	return ss.str();
}

std::string destuff_bits(std::string data)
{
	std::stringstream ss;
	char int_counter = 0;

	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (*it == '1') int_counter++;
		else int_counter = 0;
		ss << *it;

		if (int_counter == 5)
		{
			int_counter = 0;
			it++;
		}
	}

	return ss.str();
}

std::string encode(std::string data)
{
	unsigned int table[256];
	PrecomputeCRC(table);

	std::stringstream result;

	unsigned char header[11];
	int jump = 84;
	for (unsigned int i = 0; i < data.length(); i += jump)
	{
		std::string part = data.substr(i, jump);
		std::stringstream ss;
		ss << part;

		for (unsigned int i = 0; i < 11; i++)
			header[i] = std::stoi(part.substr(std::min(i * 8, part.length() - 1), 8), nullptr, 2);

		ss << std::bitset<16>(CalcCRC(header, 11, table));
		std::string stuffed_part = stuff_bits(ss.str());

		result << stuffed_part << EXIT_FLAG;
	}

	return result.str();
}

std::string decode(std::string data, bool &correct)
{
	unsigned int table[256];
	PrecomputeCRC(table);

	std::stringstream result;

	unsigned char header[11];
	int jump = 128;

	for (unsigned int i = 0; i < data.length(); )
	{
		int skip = data.find(EXIT_FLAG, i) - i;
		std::string destuffed = destuff_bits(data.substr(i, skip));

		std::string payload = destuffed.substr(0, destuffed.length() - 16);
		std::string crc = destuffed.substr(destuffed.length() - 16, 16);

		for (unsigned int i = 0; i < 11; i++)
			header[i] = std::stoi(payload.substr(std::min(i * 8, payload.length() - 1), 8), nullptr, 2);

		if (crc != std::bitset<16>(CalcCRC(header, 11, table)).to_string())
		{
			correct = false;
		}

		result << payload;
		i += skip + 8;
	}

	return result.str();
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: -[e|d] < input > output" << std::endl;
		return 1;
	}

	std::string data;
	std::getline(std::cin, data);

	if (std::string(argv[1]) == "-e")
	{
		std::string encoded = encode(data);
		std::cout << encoded << std::endl;
	}
	else if (std::string(argv[1]) == "-d")
	{
		bool correct;
		std::string decoded = decode(data, correct);
		if (correct) std::cout << decoded << std::endl;
		else std::cout << "CRC check failed" << std::endl;
	}
	else
	{
		std::cerr << "Usage: -[e|d] < input > output" << std::endl;
		return 1;
	}

	/*
	bool correct;
	std::string decoded = decode(encoded, correct);
	std::cout << "Data was " << ((data == decoded) ? "" : "not ") << "transfered correctly (CRC result: " << ((correct) ? "True" : "False") << ")" << std::endl;
	*/

	return 0;
}