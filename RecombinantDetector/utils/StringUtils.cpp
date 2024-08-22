#include "StringUtils.h"

bool StringUtils::isInteger(const std::string& str) {
    for (char character : str) {
        if (!isdigit(character)) {
            return false;
        }
    }
    return true;
}

std::string StringUtils::formatInt(const long& integer, int minDigitNum) {
    std::stringstream stream;
    std::string result;

    stream << integer;
    result = stream.str();
    minDigitNum = minDigitNum - static_cast<int>(result.length());

    while (minDigitNum > 0) {
        result = "0" + result;
        minDigitNum--;
    }

    return result;
}

std::string StringUtils::trim(std::string inStr) {
    std::string::iterator it = inStr.begin();
    while (it != inStr.end()) {
        if (isspace(*it)) {
            inStr.erase(it);
        }
        else {
            break;
        }
    }

    it = inStr.end();
    it--;
    while (it != inStr.begin()) {
        if (isspace(*it)) {
            inStr.erase(it);
            it--;
        }
        else {
            break;
        }
    }

    return inStr;
}

std::string StringUtils::deleteAllSpace(std::string inStr) {
    std::string::iterator it = inStr.begin();
    while (it != inStr.end()) {
        if (isspace(*it)) {
            inStr.erase(it);
        }
        else {
            it++;
        }
    }

    return inStr;
}

time_t StringUtils::extractDataFromString(const std::string& string) {
    std::regex pattern(R"((\d{4})([^0-9])(\d{2})([^0-9])(\d{2}))");
    std::smatch matches;
    std::time_t time_temp = -1;
    if (std::regex_search(string, matches, pattern)) {
        if (matches.size() == 6) {
            int year = std::stoi(matches[1].str());
            int month = std::stoi(matches[3].str());
            int day = std::stoi(matches[5].str());

            std::tm time_in = { 0, 0, 0, day, month - 1, year - 1900 };

            time_temp = std::mktime(&time_in);

        }
        
    }
    return time_temp;
}