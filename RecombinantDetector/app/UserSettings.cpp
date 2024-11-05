#include "UserSettings.h"
#include "../include/json.hpp"

#include <iostream>
#include <fstream>

using json = nlohmann::json;

void UserSettings::readSettings(std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Cannot open the "<< path << std::endl;
	}

	json jsonSettings;
	file >> jsonSettings;

	tryToGetDataFromSequenceName = jsonSettings["useHeaderData"];
	timeThresholdBetweenParentsAndChild = jsonSettings["timeThresholdBetweenParentsAndChild"];
	multiFileMode = jsonSettings["multiFileMode"];
	threadsCount = jsonSettings["threadsCount"];
	useSeparateParentFile = jsonSettings["useSeparateParentFile"];
	separateParentFilePath = jsonSettings["separateParentFilePath"];
	sequencesToReadLimitEnabled = jsonSettings["sequencesToReadLimitEnabled"];
	sequencesToReadLimit = jsonSettings["sequencesToReadLimit"];
	pTableFilePath = jsonSettings["pTableFilePath"];
	minLongRecombinationThreshold = jsonSettings["minLongRecombinationThreshold"];
	rejectThreshold = jsonSettings["rejectThreshold"];
	useAllSites = jsonSettings["useAllSites"];
	removeIdenticalSequences = jsonSettings["removeIdenticalSequences"];
	minIdenticalSequencesDistance = jsonSettings["minIdenticalSequencesDistance"];
	writeSkippedTriplets = jsonSettings["writeSkippedTriplets"];
	simplifiedOutput = jsonSettings["simplifiedOutput"];
	outputDirPath = jsonSettings["outputDirPath"];
	// TODO: checking the path for existence
	if (outputDirPath == "_") outputDirPath = "";
	else concatPaths();
	calculateAllBreakpoints = jsonSettings["calculateAllBreakpoints"];
	calculateNoBreakpoints = jsonSettings["calculateNoBreakpoints"];

	file.close();
}

void UserSettings::concatPaths() {
	DefaultLogFileName = outputDirPath + "/" + DefaultLogFileName;
	DefaultRecombinantsFileName = outputDirPath + "/" + DefaultRecombinantsFileName;
	DefaultLongRecombinantsFileName = outputDirPath + "/" + DefaultLongRecombinantsFileName;
	DefaultPvalHistogramFileName = outputDirPath + "/" + DefaultPvalHistogramFileName;
	DefaultSkippedTripletsFileName = outputDirPath + "/" + DefaultSkippedTripletsFileName;
}