#include "StringFunctions.hpp"

namespace StringFunctions {
	size_t ConvertStringToNumber(std::string& string)
	{
		int radix = (string[0] == '0' && string[1] == 'x' ? 16 : 10);

		char* endPtr = (char*)(string.c_str() + string.size());

		size_t num = vsh::strtoul(string.c_str(), &endPtr, radix);

		return num;
	}

	std::string ConvertIntToHexString(u32 num)
	{
		char buffer[32]{};

		// Get hex formatted string
		vsh::snprintf(buffer, sizeof(buffer), "%x", num);
		int hexLen = vsh::strlen(buffer);

		int padding = hexLen + (hexLen % 2); // Round up to even number (0x0f -> 0xff)

		// Make sure address shows all 4 bytes if it's over 1 byte.
		if (padding > 2 && padding < 8)
			padding = 8;

		vsh::snprintf(buffer, sizeof(buffer), "0x%0*X", padding, num);
		return std::string(buffer);
	}

	std::vector<u8> ConvertStringToByteBuffer(std::string& input)
	{
		std::vector<u8> buffer;

		std::string buildNumber;

		for (char character : input)
		{
			if (character != ' ')
				buildNumber += character;
			else {

				// Bug in std::vector when size_t isn't type casted, (from size_t -> u8)
				// It becomes a random huge value (268574624?) but this works in mingw cpp stdlib
				buffer.push_back( (u8)ConvertStringToNumber(buildNumber) );
				buildNumber = "";
			}
		}

		// Flush the last token if there's no trailing space
		if (!buildNumber.empty())
			buffer.push_back( (u8)ConvertStringToNumber(buildNumber) );

		return buffer;
	}
}