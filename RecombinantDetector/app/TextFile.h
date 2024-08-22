#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

#include "../utils/StringUtils.h"

class TextFile {
public:
	static const std::string DEFAULT_COLUMN_DELIM;

	explicit TextFile(std::string newFilePath);
	TextFile(const TextFile& orig);
	TextFile& operator=(const TextFile& rhs);
	virtual ~TextFile();

	virtual const std::string& getPath() const {
		return m_filePath;
	};

	virtual std::fstream* getStreamPtr() const {
		assert(m_fStreamPtr->is_open());
		return m_fStreamPtr;
	};
	virtual bool removeFile() {
		return exists() ? (remove(m_filePath.c_str()) == 0) : true;
	}

	virtual void writeLine(const std::string& line) {
		if (!m_fStreamPtr->is_open()) {
			openToWrite();
		}
		assert(!m_isReadOnly);
		(*m_fStreamPtr) << line << std::endl;
	};

	// Write an empty line into file.
	virtual void writeLine() {
		writeLine("");
	}

	virtual void write(const std::string& str) {
		if (!m_fStreamPtr->is_open()) {
			openToWrite();
		}
		assert(!m_isReadOnly);
		(*m_fStreamPtr) << str;
	};

	virtual std::string getLine();

	virtual bool isOpen() const {
		return (m_fStreamPtr != nullptr && m_fStreamPtr->is_open());
	}

	virtual void openToRead();
	virtual void openToWrite();
	virtual void close();
	virtual bool exists();
	virtual bool isStreamReadable() const;
	virtual std::vector<std::string> readAllLines();


private:

	static bool isInvalidPathChar(char testedChar) {
		for (size_t i = 0; i < INVALID_CHARS_COUNT; i++) {
			if (testedChar == INVALID_PATH_CHARS[i]) {
				return true;
			}
		}
		return false;
	};

	static bool isPathSeparator(char testedChar) {
		for (size_t i = 0; i < PATH_SEPARATORS_COUNT; i++) {
			if (testedChar == PATH_SEPARATORS[i]) {
				return true;
			}
		}
		return false;
	};

	static const char INVALID_PATH_CHARS[];
	static const size_t INVALID_CHARS_COUNT;

	static const char PATH_SEPARATORS[];
	static const size_t PATH_SEPARATORS_COUNT;

private:

	std::string m_filePath;
	std::fstream* m_fStreamPtr;

	bool m_isReadOnly;
};
