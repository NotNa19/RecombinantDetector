#include "TextFile.h"

#include <utility>

#include "App.h"

const char TextFile::INVALID_PATH_CHARS[] = { '\"', '*', '<', '>', '?', '|' };
const size_t TextFile::INVALID_CHARS_COUNT
= sizeof(INVALID_PATH_CHARS) / sizeof(char);

const char TextFile::PATH_SEPARATORS[] = { '\\', '/' };
const size_t TextFile::PATH_SEPARATORS_COUNT
= sizeof(PATH_SEPARATORS) / sizeof(char);

const std::string TextFile::DEFAULT_COLUMN_DELIM = "\t";

TextFile::TextFile(std::string newFilePath) : m_filePath(std::move(newFilePath)) {
    m_fStreamPtr = new std::fstream;
    m_isReadOnly = false;

    try {
        if (m_filePath.length() <= 0) throw (m_filePath);

        for (char pathChar : m_filePath) {
            if (isInvalidPathChar(pathChar)) {
                throw (m_filePath);
            }
        }

    }
    catch (const std::string& invalidFilePath) {
        App::instance() << "Invalid file name: \"" << invalidFilePath << "\".\n";
        App::instance().showError(true, true); // show error & exit
    }
}

TextFile::TextFile(const TextFile& orig) = default;

TextFile& TextFile::operator=(const TextFile & rhs) {
    if (this != &rhs) {
        this->m_filePath = rhs.m_filePath;
        this->m_fStreamPtr = rhs.m_fStreamPtr;
        this->m_isReadOnly = rhs.m_isReadOnly;
    }

    return *this;
}

TextFile::~TextFile() {
    if (m_fStreamPtr) {
        delete m_fStreamPtr;
        m_fStreamPtr = nullptr;
    }
}

std::string TextFile::getLine() {
    assert(m_isReadOnly && m_fStreamPtr->is_open());

    std::string newLine;

    std::istream::sentry se(*m_fStreamPtr);

    std::streambuf* buffer = m_fStreamPtr->rdbuf();

    while (true) {
        int newChar = buffer->sbumpc();

        switch (newChar) {
        case '\r':
            newChar = buffer->sgetc();
            if (newChar == '\n')
                buffer->sbumpc();
            return newLine;

        case '\n':
        case EOF:
            return newLine;

        default:
            newLine += (char)newChar;
        }
    }
}

void TextFile::openToRead() {
    if (m_fStreamPtr->is_open()) {
        m_fStreamPtr->close();
    }

    m_fStreamPtr->open(m_filePath.c_str(), std::ios::in);
    m_isReadOnly = true;
}

void TextFile::openToWrite() {
    if (m_fStreamPtr->is_open()) {
        m_fStreamPtr->close();
    }

    m_fStreamPtr->open(m_filePath.c_str(), std::ios::out);
    m_isReadOnly = false;
}

void TextFile::close() {
    if (m_fStreamPtr->is_open()) {
        m_fStreamPtr->close();
    }
    m_isReadOnly = false;
}

bool TextFile::exists() {
    if (m_filePath.length() <= 0) {
        return false;
    }

    openToRead();

    if (isStreamReadable()) {
        close();
        return true;
    }
    else {
        return false;
    }
}

bool TextFile::isStreamReadable() const {
    return (m_fStreamPtr->is_open() && !m_fStreamPtr->fail() && !m_fStreamPtr->eof());
}

std::vector<std::string> TextFile::readAllLines() {
    std::vector<std::string> lines;
    std::string newLine;

    assert(exists());
    openToRead();

    while (isStreamReadable()) {
        newLine = getLine();
        newLine = StringUtils::trim(newLine);

        if (newLine.length() > 0) {
            lines.push_back(newLine);
        }
    }

    close();
    return lines;
}
