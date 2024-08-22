#include "FastaReader.h"

#include <stdexcept>
#include <filesystem>

#include "App.h"
#include "UserSettings.h"

FastaReader::FastaReader(const std::string& newFilePath, Type newFileType)
	: FastaFile(newFilePath), m_fileType(newFileType) {
}

void FastaReader::detectFileType() {
	/* This method should not be called if the file type has been known */
	if (m_fileType != Type::Unknown) {
		return;
	}

	std::string firstString;

	assert(FastaFile::exists());
	FastaFile::openToRead();

	std::fstream* fileStream = FastaFile::getStreamPtr();
	(*fileStream) >> firstString;

	if (FastaFile::isStreamReadable() && firstString[0] == '>') {
		m_fileType = Type::FASTA;
	}

	FastaFile::close();
}

std::vector<SequencePtr> FastaReader::read() {
	detectFileType();

	switch (m_fileType) {
	case Type::FASTA:
		return FastaFile::read();
	default:
		throw std::runtime_error("Invalid sequence file format.");
	}
}

void FastaReader::write(const Alignment& alignment) {
	switch (m_fileType) {
	case Type::FASTA:
		FastaFile::write(alignment);
		return;
	default:
		App::instance() << "Invalid alignment file format.\n";
		App::instance().showError(true, true);
		return;
	}
}


FastaFile::FastaFile(const std::string& newFilePath) : TextFile(newFilePath) {
}

std::vector<SequencePtr> FastaFile::read() {
	std::vector<SequencePtr> sequenceList;

	assert(exists());
	openToRead();

	size_t seqCount = 0;
	std::string seqName;
	std::string seqString;

	bool forceStop = false;
	size_t nodeToRead = UserSettings::instance().sequencesToReadLimitEnabled ? UserSettings::instance().sequencesToReadLimit : std::numeric_limits<size_t>::max();
	// Read through each line of the file
	while (isStreamReadable() && !forceStop) {
		std::string line = StringUtils::trim(getLine());

		// If reach end of file or a new seqName number is found
		if (!isStreamReadable() || (line.length() > 0 && line[0] == '>')) {
			if (line.length() > 0 && line[0] != '>') {
				// This means reaching the end of file
				seqString += StringUtils::deleteAllSpace(line);
			}

			// If the previous sequence has not been stored into the m_alignment
			if (seqString.length() > 0) {
				/* Check if its seqName is valid */
				if (seqName.length() <= 0) {
					App::instance() << "Invalid seqName at sequence "
						<< seqCount + 1 << " in file "
						<< getPath() << ".\n";
					App::instance().showError(true, true);
				}
				auto sequence = Sequence::create(seqName, seqString);
				sequenceList.push_back(sequence);
				if (sequenceList.size() > nodeToRead)
					forceStop = true;
			}

			if (line.length() > 0 && line[0] == '>') {
				seqName = line.substr(1, line.length() - 1);
				seqString = "";
			}

		}
		else {
			seqString += StringUtils::deleteAllSpace(line);
		}
	};
	if (forceStop) {
		App::instance() << "Reading: stop after " << nodeToRead << " seqs";
		App::instance().showLog(true);
	}
	close();
	return sequenceList;
}

void FastaFile::write(const Alignment& alignment) {
	if (exists()) {
		removeFile();
	}

	openToWrite();
	write(alignment, getStreamPtr());
	close();
}

void FastaFile::write(const Alignment& alignment,
	std::ostream* streamPtr) {
	auto sequenceList = alignment.getAllUsedSequences();
	for (const auto& sequence : sequenceList) {
		(*streamPtr) << ">" << sequence->name() << std::endl
			<< sequence->toString() << std::endl;

	}
	(*streamPtr) << std::endl;
}

void FastaFile::write(const std::unordered_map<std::string, SequencePtr>& clades) {
	if (exists()) removeFile();

	openToWrite();
	for (const auto& [_, sequence] : clades) {
		(*getStreamPtr()) << ">" << sequence->name() << std::endl
			<< sequence->toString() << std::endl;

	}
	(*getStreamPtr()) << std::endl;

	close();
}

DirectoryReader::DirectoryReader(const std::string& directoryPath)
	: m_dirPath(directoryPath) {
}

std::vector<SequencePtr> DirectoryReader::read() {
	std::vector<SequencePtr> result;

	for (auto& p : std::filesystem::directory_iterator(m_dirPath)) {
		std::string filePath = p.path().string();
		std::string ext = p.path().extension().string();
		if (ext != ".fasta" && ext != ".FASTA") {
			continue;
		}
		App::instance() << "Reading: " << filePath << " ...";
		App::instance().showLog(true);
		FastaReader reader(filePath, FastaReader::Type::FASTA);
		auto seqs = reader.read();

		result.reserve(result.size() + seqs.size());
		result.insert(result.end(), seqs.begin(), seqs.end());
	}
	return result;
}