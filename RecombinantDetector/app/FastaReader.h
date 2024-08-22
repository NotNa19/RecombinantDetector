#ifndef FastaReader_H
#define    FastaReader_H

#include "TextFile.h"

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "../core/Alignment.h"
#include "../core/Sequence.h"

class FastaFile : public TextFile {
public:
	explicit FastaFile(const std::string& newFilePath);

	virtual std::vector<SequencePtr> read();

	virtual void write(const Alignment& alignment);

	void write(const std::unordered_map<std::string, SequencePtr>& clades);

	static void write(const Alignment& alignment, std::ostream* streamPtr);
};

class FastaReader : public FastaFile {
public:

	enum class Type {
		Unknown, FASTA
	};

	explicit FastaReader(const std::string& newFilePath, Type newFileType);

	bool exists() override {
		return FastaFile::exists();
	};

	virtual const std::string& getPath() {
		return FastaFile::getPath();
	};

	virtual void setFileType(Type newType) {
		assert(m_fileType == Type::Unknown);
		m_fileType = newType;
	};

	std::vector<SequencePtr> read() override;

	void write(const Alignment& alignment) override;

private:
	void detectFileType();

private:
	Type m_fileType;
};

class DirectoryReader {

public:
	DirectoryReader(const std::string& directoryPath);

	std::vector<SequencePtr> read();
private:

private:
	std::string m_dirPath;
};
#endif    /* FastaReader_H */

