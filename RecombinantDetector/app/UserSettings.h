#pragma once
#include <string>
#include <filesystem>

// TODO: use json to store settings
// TODO: replace string paths with filsystem::filename
struct UserSettings
{
public:
	UserSettings() = default;
	UserSettings(const UserSettings& other) = delete;

	void readSettings(std::string& path);

	static UserSettings& instance() {
		static UserSettings instance;
		return instance;
	}

	bool tryToGetDataFromSequenceName = false;
	// 1 month in sec
	time_t timeThresholdBetweenParentsAndChild = 2678400;

	// For now use -dir parameter instead of this flag
	bool multiFileMode = 0;

	size_t threadsCount = 6;
	bool useSeparateChildFile = 0;
	std::string separateChildFilePath = "";
	bool sequencesToReadLimitEnabled = 0;
	size_t sequencesToReadLimit = 100;

	std::string pTableFilePath = "";
	size_t minLongRecombinationThreshold = 100;
	double rejectThreshold = 0.05;

	// use all sites of the sequence, not only polymorphic
	bool useAllSites = false;

	/* Remove one of the sequences if the distance between them is less then
	minIdenticalSequencesDistance and isIdenticalSeqRemoved flag is turned on*/
	bool removeIdenticalSequences = false;
	size_t minIdenticalSequencesDistance = 0;

	// Write skipped triplets in the file
	bool writeSkippedTriplets = false;

	// Simplified output of the final recombinant table
	bool simplifiedOutput = true;

	std::string outputDirPath = "";

	// By default, we calculate the best breakpoints. But it is possible to change this using these two flags.
	bool calculateAllBreakpoints = false;
	bool calculateNoBreakpoints = false;

	// Const parameters
	// Default file names
	std::string DefaultLogFileName = "RecDetector.log";
	std::string DefaultRecombinantsFileName = "results.csv";
	std::string DefaultLongRecombinantsFileName = "longRecs.log";
	std::string DefaultPvalHistogramFileName = "pvalHist.log";
	std::string DefaultSkippedTripletsFileName = "skippedTriplets.log";

	// The size of the histogram of P-values
	const int PvalHistogramSize = 41; // 0 to 40

	// The rate (in seconds) at which the progress counter is updated.
	const long UpdateMonitorInSec = 2;

private:
	void concatPaths();
};
