#include "PTableFile.h"
#include "PTableFile.h"

#include "../utils/Utils.h"
#include "UserSettings.h"

#include "App.h"

const char PTableFile::FILE_MARKER[] = "P-table";

const bool PTableFile::exists() {
	if (filePath.length() <= 0) {
		return false;
	}

	fstream stream;
	stream.open(filePath.c_str(), ios::in);

	if (stream.is_open() && !stream.fail() && !stream.eof()) {
		stream.close();
		return true;
	}
	else {
		return false;
	}
}

const bool PTableFile::save(const PTable& pTable) {
	try {
		fstream binaryFile(filePath.c_str(), ios::out | ios::binary);

		/* Write the marker */
		binaryFile.write((char*)&FILE_MARKER, sizeof(FILE_MARKER));

		/* Save the sizeof(int) in the current system (this may help to indicate
		 * the system architecture where the P-value table file is created) */
		int intSize = sizeof(int);
		binaryFile.write((char*)&intSize, sizeof(int));

		int mSize = pTable.getMSize();
		int nSize = pTable.getNSize();
		int kSize = pTable.getKSize();
		binaryFile.write(reinterpret_cast<char*> (&mSize), sizeof(int));
		binaryFile.write(reinterpret_cast<char*> (&nSize), sizeof(int));
		binaryFile.write(reinterpret_cast<char*> (&kSize), sizeof(int));

		float* dataPtr = pTable.getDataPtr();
		binaryFile.write(reinterpret_cast<char*> (dataPtr), pTable.getNumStoredVals() * sizeof(float));

		binaryFile.close();
		return true;

	}
	catch (...) {
		return false;
	}
}

const PTableFile::ReadResult PTableFile::tryLoadInto(PTable& pTable) {
	int mSize, nSize, kSize, memNeeded;

	fstream binaryFile(filePath.c_str(), ios::in | ios::binary);
	if (!binaryFile.is_open() || binaryFile.fail() || binaryFile.eof()) {
		return INVALID_FILE;
	}

	try {
		char* marker = new char[sizeof(FILE_MARKER)];
		binaryFile.read(marker, sizeof(FILE_MARKER));
		std::string strMarker(marker);
		std::string strFileMarker(FILE_MARKER);
		delete marker;

		if (strMarker.compare(strFileMarker) != 0)
			return INVALID_FILE;


		int intSize;
		binaryFile.read((char*)&intSize, sizeof(int));
		if (intSize != sizeof(int))
			return WRONG_ARCH;


		binaryFile.read((char*)&mSize, sizeof(int));
		binaryFile.read((char*)&nSize, sizeof(int));
		binaryFile.read((char*)&kSize, sizeof(int));

		char lineBreak = ' ';
		if (filePath.length() > 40) lineBreak = '\n';


		App::instance()
			<< "Loading P-value table from file" << lineBreak
			<< "\"" << filePath << "\"\n"
			<< App::DEFAULT_INDENT << "Table size : "
			<< mSize << " * " << nSize << " * " << kSize << endl;
		App::instance().showLog(true);


	}
	catch (...) {
		return INVALID_FILE;
	}

	try {

		pTable.initialize(mSize, nSize, kSize);
		float* dataPtr = pTable.getDataPtr();
		binaryFile.read(reinterpret_cast<char*> (dataPtr),
			pTable.getNumStoredVals() * sizeof(float));

		if (!binaryFile) {
			return FILE_CORRUPT;
		}
		binaryFile.close();
	}
	catch (...) {
		return FILE_CORRUPT;
	}

	return SUCCESS;
}

PTableFile::ReadResult PTableFile::Load(PTable& pTable) {
	PTableFile file(UserSettings::instance().pTableFilePath);
	ReadResult loadResult = file.tryLoadInto(pTable);
	return loadResult;
}
