#pragma once
#include "Utils/Types.hpp"

#include <string>
#include <vsh/stdc.hpp>


namespace StringFunctions {
	size_t ConvertStringToNumber(std::string& string);
	
	std::string ConvertIntToHexString(u32 num);
	
	std::vector<u8> ConvertStringToByteBuffer(std::string& input);

	bool IsInString(const std::string matchString, const std::string target);
}