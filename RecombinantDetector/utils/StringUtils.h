#pragma once

#include <string>
#include <sstream>
#include <regex>

namespace StringUtils {
	bool isInteger(const std::string& str);

	/**
	 * Parse the given integer to string. If the number of digits in the given
	 * integer less than minDigitNum, some extra 0's will added to the
	 * left side of this integer.
	 */
	std::string formatInt(const long& integer, int minDigitNum);

	//  Delete white spaces at the beginning and the end of the given string
	std::string trim(std::string inStr);
	std::string deleteAllSpace(std::string inStr);

	time_t extractDataFromString(const std::string& name);
};


