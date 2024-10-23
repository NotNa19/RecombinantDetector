#include "UserSettings.h"
#include <iostream>
#include <fstream>

void UserSettings::readSettings(std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Cannot open the user_settings.rec" << std::endl;
	}
	std::string ignore;
	file >> ignore >> tryToGetDataFromSequenceName;
	file >> ignore >> timeThresholdBetweenParentsAndChild;
	file >> ignore >> multiFileMode;
	file >> ignore >> threadsCount;
	file >> ignore >> useSeparateParentFile;
	file >> ignore >> separateParentFilePath;
	file >> ignore >> sequencesToReadLimitEnabled;
	file >> ignore >> sequencesToReadLimit;
	file >> ignore >> pTableFilePath;
	file >> ignore >> minLongRecombinationThreshold;
	file >> ignore >> rejectThreshold;
	file >> ignore >> useAllSites;
	file >> ignore >> removeIdenticalSequences;
	file >> ignore >> minIdenticalSequencesDistance;
	file >> ignore >> writeSkippedTriplets;
	file >> ignore >> simplifiedOutput;
	file >> ignore >> outputDirPath;
	// TODO: checking the path for existence
	if (outputDirPath == "_") outputDirPath = "";
	else concatPaths();
	file >> ignore >> calculateAllBreakpoints;
	file >> ignore >> calculateNoBreakpoints;

	file.close();
}

void UserSettings::concatPaths() {
	DefaultLogFileName = outputDirPath + "/" + DefaultLogFileName;
	DefaultRecombinantsFileName = outputDirPath + "/" + DefaultRecombinantsFileName;
	DefaultLongRecombinantsFileName = outputDirPath + "/" + DefaultLongRecombinantsFileName;
	DefaultPvalHistogramFileName = outputDirPath + "/" + DefaultPvalHistogramFileName;
	DefaultSkippedTripletsFileName = outputDirPath + "/" + DefaultSkippedTripletsFileName;
}