#ifndef RecombinantDetector_H
#define	RecombinantDetector_H

#include <cassert>
#include <string>
#include <mutex>
#include "Run.h"
#include "../FastaReader.h"
#include "../PTableFile.h"
#include "../UserSettings.h"
#include "../../core/Triplet.h"
#include "../../core/TripletPool.h"
#include "../../core/AlignmentDescriptor.h"


class RecombinantDetector : public Run {
public:
	explicit RecombinantDetector(int argc, char** argv);

	virtual ~RecombinantDetector();

	virtual bool isLogFileSupported() const {
		return true;
	};

	virtual Mode getMode() const {
		return Mode::RecombinantDetection;
	};

	virtual void parseCmdLine();

	virtual void perform();


private:
	struct IntermediateThreadsData {
		IntermediateThreadsData(size_t iThreadCount, std::vector<SequencePtr>& iChildSequences, std::vector<SequencePtr>& iParentSequences) :threadCount(iThreadCount), numsSkipped(threadCount),
			numsComputedExactly(threadCount),
			numsApproximated(threadCount),
			numsRecombinantTriplets(threadCount),
			numsTripletsSkippedByTime(threadCount),
			performedOuterLoops(threadCount),
			minPvals(threadCount),
			tripletPools(threadCount),
			containerIsLocked(threadCount),
			childSequences(iChildSequences),
			parentSequences(iParentSequences) {

			// we can experiment with these values
			if (iChildSequences.size() > threadCount * 4)
				chunksCount = threadCount * 4;

			for (size_t i = 0; i < threadCount; i++) {
				if (UserSettings::instance().calculateAllBreakpoints) {
					tripletPools[i].setStorageMode(TripletPool::StorageMode::AllTriplets);
				}
				else {
					tripletPools[i].setStorageMode(TripletPool::StorageMode::BestTriplet);
				}

				minPvals[i] = 1.0;
				performedOuterLoops[i] = 0.0;
			}
		};

		size_t threadCount;
		size_t chunksCount;
		size_t processedChunks = 0;

		std::vector<size_t> numsSkipped;
		std::vector<size_t> numsComputedExactly;
		std::vector<size_t> numsApproximated;
		std::vector<size_t> numsRecombinantTriplets;
		std::vector<size_t> numsTripletsSkippedByTime;
		std::vector<size_t> performedOuterLoops;
		std::vector<double> minPvals;
		std::vector<TripletPool> tripletPools;

		// shared resource
		std::vector<bool> containerIsLocked;

		std::vector<SequencePtr>& childSequences;
		std::vector<SequencePtr>& parentSequences;

		std::mutex guard;

	};

	RecombinantDetector(const RecombinantDetector& orig) = delete;

	RecombinantDetector& operator=(const RecombinantDetector& rhs) = delete;

	void dataInfo();
	void setup();
	void analyze();
	void showProgress(double currentLoop, bool isFinish, std::vector<size_t>& numsRecombinantTriplets, std::vector<double>& minPvals, std::vector<size_t>& numsSkippedByHeader) const;
	void displayResult();

	void process(IntermediateThreadsData& threadsData, size_t chunkIdx);
private:
	bool m_readFromDir = false;
	bool m_tryToGetDataFromSequenceName = false;

	TripletPool m_tripletPool;

	AlignmentDescriptor m_alignmentDescriptor;

	PTableFile* m_pTableFile;

	double m_minPVal;

	size_t m_numTripletsForStatCorrection;

	size_t m_numRecombinantTriplets;
	size_t m_numComputedExactly;
	size_t m_numApproximated;
	size_t m_numSkipped;
	size_t m_numTripletsSkippedByTime;

	TextFile* m_fileSkippedTriplets;
	TextFile* m_fileRecombinants;
	TextFile* m_fileLongRecombinants;
};

#endif	/* RecombinantDetector_H */

