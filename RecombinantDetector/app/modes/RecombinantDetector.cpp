#include "RecombinantDetector.h"

#include <numeric> 

#include "../UserSettings.h"
#include "../../utils/ThreadPool.h"
#include "../../utils/numeric_types.h"

RecombinantDetector::RecombinantDetector(int argc, char** argv)
	: Run(argc, argv),
	m_tripletPool(), m_alignmentDescriptor(this->m_alignment) {
	App::instance().startProgram("Full Run");

	m_fileSkippedTriplets = nullptr;
	m_fileLongRecombinants = nullptr;
}

RecombinantDetector::~RecombinantDetector() {
	delete m_fileSkippedTriplets;
	delete m_fileLongRecombinants;
}

void RecombinantDetector::parseCmdLine() {
	std::vector<SequencePtr> parentSeqs;
	std::vector<SequencePtr> childSeqs;

	int argNum = getRunArgsNum();

	if (argNum < 0) {
		App::instance() << "Not enough parameter for full run.\n";
		App::instance().showError(true, true);
	}

	m_pTableFile = nullptr;
	for (int i = 2 + argNum; i < m_argCount; i++) {
		string arg = m_argVector[i];
		if (arg == "-dir") {
			m_readFromDir = true;
		}
	}

	if (UserSettings::instance().tryToGetDataFromSequenceName) {
		m_tryToGetDataFromSequenceName = true;
	}

	if (m_pTableFile != nullptr && !m_pTableFile->exists()) {
		App::instance() << "The file \"" << m_pTableFile->getFilePath()
			<< "\" does not exist.\n";
		App::instance().showError(true, true);
	}

	if (m_readFromDir) {
		string dirPath = m_argVector[3];
		DirectoryReader dirReader(dirPath);
		parentSeqs = dirReader.read();
	}
	else {
		string filePath = m_argVector[2];
		FastaReader parentFile(filePath, FastaReader::Type::Unknown);
		if (!parentFile.exists()) {
			App::instance() << "File \"" << filePath << "\" not found.\n";
			App::instance().showError(true, true);
		}
		parentSeqs = parentFile.read();

		if (argNum >= 2) {
			argNum = 2;
			filePath = m_argVector[3];
			// THE CHILD PATH IS SPECIFIED MANUALLY.
			FastaReader childFile(filePath, FastaReader::Type::Unknown);
			if (!childFile.exists()) {
				App::instance() << "File \"" << filePath << "\" not found.\n";
				App::instance().showError(true, true);
			}
			childSeqs = childFile.read();
		}
	}

	if (m_tryToGetDataFromSequenceName) {
		size_t validDatas = 0;
		std::vector<SequencePtr> newParentSeqs;
		newParentSeqs.reserve(parentSeqs.size());
		for (const auto& seq : parentSeqs) {
			if (seq->data() != -1) {
				validDatas++;
				newParentSeqs.push_back(seq);
			}
		}
		App::instance() << "Sequences with correct data in header: " << validDatas << "\nTotal sequences: " <<
			parentSeqs.size() << "\nPercentage of correct dates: " << static_cast<double>(validDatas) / parentSeqs.size();
		App::instance().showLog(true);
		parentSeqs = newParentSeqs;
	}

	if (childSeqs.empty()) {
		m_alignment.addSequences(parentSeqs, true, true);
	}
	else {
		m_alignment.addSequences(parentSeqs, true, false);
		m_alignment.addSequences(childSeqs, false, true);
	}

	if (m_readFromDir)
		deleteCmdLineArgs(argNum + 3);
	else
		deleteCmdLineArgs(argNum + 1);
	Run::parseCmdLine();
}

void RecombinantDetector::perform() {
	// Process common data. The m_alignment is also modified (cut, shrink, etc.) at this step.
	Run::perform();

	dataInfo();
	setup();
	loadPTable(m_pTableFile);

	App::instance() << App::SEPARATOR << endl;
	App::instance().showLog(true);

	analyze();
	m_tripletPool.writeToFile(m_fileRecombinants);

	App::instance() << App::SEPARATOR << endl;
	App::instance().showLog(true);

	displayResult();
	App::instance() << App::SEPARATOR << endl;
	App::instance().showLog(true);
	savePValHistogram('\t');

	/* Close all files */
	if (m_fileSkippedTriplets)
		m_fileSkippedTriplets->close();
	if (m_fileRecombinants)
		m_fileRecombinants->close();
	if (m_fileLongRecombinants != nullptr)
		m_fileLongRecombinants->close();
}

void RecombinantDetector::dataInfo() {
	App::instance()
		<< "Using " << m_alignment.getParentPool().activeSequencesSize()
		<< " sequences as parents.\n"
		<< "Using " << m_alignment.getChildPool().activeSequencesSize()
		<< " sequences as children.\n";
	App::instance().showLog(true);
}

void RecombinantDetector::setup() {
	Triplet::setAcceptApproxPVal(true);
	Triplet::setLongRecombinantThreshold(UserSettings::instance().minLongRecombinationThreshold);

	this->m_numTripletsForStatCorrection = m_alignmentDescriptor.getTripletCounts()->all;

	StatisticalUtils::setSampleNumForPValCorrection(m_numTripletsForStatCorrection);
	App::instance() << "Need a p-value of "
		<< UserSettings::instance().rejectThreshold / m_numTripletsForStatCorrection
		<< " to survive multiple comparisons correction.\n";
	App::instance().showLog(true);

	if (UserSettings::instance().calculateAllBreakpoints) {
		m_tripletPool.setStorageMode(TripletPool::StorageMode::AllTriplets);
	}
	else {
		m_tripletPool.setStorageMode(TripletPool::StorageMode::BestTriplet);
	}

	/* Prepare file names */
	const auto& settings = UserSettings::instance();
	string recombinantsFileName = settings.DefaultRecombinantsFileName;
	string longRecombinantsFileName = settings.DefaultLongRecombinantsFileName;
	string skippedTripletFileName = settings.DefaultSkippedTripletsFileName;

	/* 3s.skipped */
	m_fileSkippedTriplets = nullptr;
	if (UserSettings::instance().writeSkippedTriplets) {
		m_fileSkippedTriplets = new TextFile(skippedTripletFileName);

		App::instance()
			<< "Skipped triplets will be recorded to the file \""
			<< m_fileSkippedTriplets->getPath() << "\".\n";

		if (m_fileSkippedTriplets->exists()) {
			m_fileSkippedTriplets->removeFile();
		}
		else {
			App::instance().showLog(true);
		}
	}

	/* 3s.rec */
	m_fileRecombinants = nullptr;

	m_fileRecombinants = new TextFile(recombinantsFileName);

	App::instance() << "All recombinant triplets will be recorded to the file \""
		<< m_fileRecombinants->getPath() << "\".\n";

	if (m_fileRecombinants->exists())
	{
		m_fileRecombinants->removeFile();
	}
	else
	{
		App::instance().showLog(true);
	}

	/* 3s.longRec */
	m_fileLongRecombinants = new TextFile(longRecombinantsFileName);
	App::instance()
		<< "Long recombinants will be recorded to the file \""
		<< m_fileLongRecombinants->getPath() << "\".\n";

	if (m_fileLongRecombinants->exists()) {
		m_fileLongRecombinants->removeFile();
	}
	else {
		App::instance().showLog(true);
	}
}

void RecombinantDetector::showProgress(double currentLoop, bool isFinish, std::vector<size_t>& numsRecombinantTriplets, std::vector<double>& minPvals, std::vector<size_t>& numsSkippedbyHeader) const {
	char strBuf[200];

	if (UserSettings::instance().calculateAllBreakpoints) {
		sprintf(strBuf,
			"     %s       %e       %10.0llu            %lu",
			App::instance().getElapsedTime().c_str(),
			m_minPVal, m_numRecombinantTriplets,
			m_tripletPool.getLongestMinRecLength());
	}
	else if (m_tryToGetDataFromSequenceName) {
		size_t totalRecombinants = 0;
		size_t totalSkippedByHeader = 0;
		for (size_t tripletsCount : numsRecombinantTriplets) totalRecombinants += tripletsCount;
		for (size_t tripletsCount : numsSkippedbyHeader) totalSkippedByHeader += tripletsCount;

		sprintf(strBuf,
			"     %s       %e       %10.0llu            %lu",
			App::instance().getElapsedTime().c_str(),
			*std::min_element(minPvals.begin(), minPvals.end()), totalRecombinants, totalSkippedByHeader);
	}
	else {
		size_t totalRecombinants = 0;
		for (size_t tripletsCount : numsRecombinantTriplets) totalRecombinants += tripletsCount;

		sprintf(strBuf,
			"     %s       %e       %10.0llu",
			App::instance().getElapsedTime().c_str(),
			*std::min_element(minPvals.begin(), minPvals.end()), totalRecombinants);
	}
	App::instance() << strBuf;

	if (isFinish) {
		App::instance().finishCounting();
	}
	else {
		App::instance().count(currentLoop);
	}
}

void RecombinantDetector::analyze() {
	if (UserSettings::instance().calculateAllBreakpoints) {
		App::instance()
			<< "                                                Recombinant      Longest Recombinant\n"
			<< "Progress    Time Elapsed    Minimum P-Value    Triplets Found          Segment\n";
	}
	else if (m_tryToGetDataFromSequenceName) {
		App::instance()
			<< "                                                Recombinant        Skipped Triplets\n"
			<< "Progress    Time Elapsed    Minimum P-Value    Triplets Found       by header data\n ";
	}
	else {
		App::instance()
			<< "                                                Recombinant\n"
			<< "Progress    Time Elapsed    Minimum P-Value    Triplets Found\n";
	}
	App::instance().showLog(false);

	auto childSequences = m_alignment.getActiveChildren();
	auto parentSequences = m_alignment.getActiveParents();

	size_t activeChildNum = childSequences.size();
	size_t activeParentNum = parentSequences.size();
	double totalOuterLoops = static_cast<double> (activeChildNum)
		* static_cast<double> (activeParentNum);
	App::instance().initCounter("", 0, totalOuterLoops);

	// This flag indicates if the progressing should be stopped
	bool isStopped = false;

	size_t threadCount = UserSettings::instance().threadsCount;
	if (threadCount > childSequences.size()) threadCount = childSequences.size();

	ThreadPool pool(threadCount + 1);
	std::vector<std::future<void>> tasks;

	// IntermediateThreadsData also manages chunk sizes for multithreading
	IntermediateThreadsData threadsData(threadCount, childSequences, parentSequences);
	// Managing task
	tasks.emplace_back(pool.enqueue([this, &threadsData] {
		time_t lastTime =
			time(nullptr) - UserSettings::instance().UpdateMonitorInSec - 1;

		while (true) {
			time_t currentTime = time(nullptr);
			if (currentTime - lastTime >= UserSettings::instance().UpdateMonitorInSec) {
				double outerLoops = std::accumulate(threadsData.performedOuterLoops.begin(), threadsData.performedOuterLoops.end(), 0);
				showProgress(outerLoops, false, threadsData.numsRecombinantTriplets, threadsData.minPvals, threadsData.numsTripletsSkippedByTime);
				lastTime = currentTime;

				if (threadsData.processedChunks == threadsData.chunksCount) return;
			}
		}
		}));

	for (int i = 0; i < threadsData.chunksCount; ++i) {
		tasks.emplace_back(
			pool.enqueue([this, i, &threadsData]
				{
					this->process(threadsData, i);
				})
		);
	}

	for (auto&& task : tasks) task.get();

	m_numRecombinantTriplets = 0;
	m_numComputedExactly = 0;
	m_numApproximated = 0;
	m_numSkipped = 0;
	m_numTripletsSkippedByTime = 0;

	for (size_t i = 0; i < threadCount; i++) {
		m_numSkipped += threadsData.numsSkipped[i];
		m_numComputedExactly += threadsData.numsComputedExactly[i];
		m_numApproximated += threadsData.numsApproximated[i];
		m_numRecombinantTriplets += threadsData.numsRecombinantTriplets[i];
		m_numTripletsSkippedByTime += threadsData.numsTripletsSkippedByTime[i];
		for (const auto& [child, seqs] : threadsData.tripletPools[i].savedTriplets) {
			for (const auto& seq : seqs)
				m_tripletPool.saveTriplet(child, seq);
		}
	}
	m_minPVal = *std::min_element(threadsData.minPvals.begin(), threadsData.minPvals.end());

	if (!UserSettings::instance().calculateNoBreakpoints)
		for (const auto& child : childSequences) m_tripletPool.seekBreakPointPairs(child);

	/* Progressing finished */
	showProgress(0.0, true, threadsData.numsRecombinantTriplets, threadsData.minPvals, threadsData.numsTripletsSkippedByTime);

	App::instance().showLog(true);
}

void RecombinantDetector::process(IntermediateThreadsData& threadData, size_t chunkIdx) {
	threadData.guard.lock();
	// TODO: add entry to the technical log file about starting
	size_t freeContainerId = ULong::NOT_SET;
	for (size_t i = 0; i < threadData.containerIsLocked.size(); i++) {
		if (!threadData.containerIsLocked[i]) {
			threadData.containerIsLocked[i] = true;
			freeContainerId = i;
			break;
		}
	}
	// TODO: add entry to the technical log file
	if (freeContainerId > threadData.threadCount)
		std::cout << "WARNING: freeContainerId > threadData.threadCount " << freeContainerId;
	auto i = freeContainerId;
	threadData.containerIsLocked[i] = true;
	threadData.guard.unlock();

	size_t chunkSize = threadData.childSequences.size() / threadData.chunksCount;
	size_t begin = chunkIdx * chunkSize;
	size_t end = (chunkIdx < threadData.chunksCount - 1) ? ((chunkIdx + 1) * chunkSize - 1) : threadData.childSequences.size();

	for (size_t childIdx = begin; childIdx < end; childIdx++) {
		const auto& child = threadData.childSequences.at(childIdx);

		for (const auto& dad : threadData.parentSequences) {
			if (dad == child) {
				continue;
			}
			if (m_tryToGetDataFromSequenceName && child->isOlderThan(*dad)) {
				threadData.numsTripletsSkippedByTime[i] += threadData.parentSequences.size();
				continue;
			}
			threadData.performedOuterLoops[i] += 1.0;

			for (const auto& mum : threadData.parentSequences) {
				if (mum == dad || mum == child) {
					continue;
				}
				if (m_tryToGetDataFromSequenceName && child->isOlderThan(*mum)) {
					threadData.numsTripletsSkippedByTime[i] += 1;
					continue;
				}

				auto triplet = threadData.tripletPools[i].newTriplet(child, dad, mum);

				if (!triplet->hasPVal()) {
					if (m_fileSkippedTriplets) {
						m_fileSkippedTriplets->writeLine(triplet->info());
					}
					threadData.numsSkipped[i]++;
					continue;
				}

				if (triplet->hasExactPVal()) {
					threadData.numsComputedExactly[i]++;
				}
				else {
					threadData.numsApproximated[i]++;
				}

				double pValue = triplet->getPValue();
				addPValIntoHistogram(pValue);
				if (pValue < threadData.minPvals[i]) {
					threadData.minPvals[i] = pValue;
				}

				auto correctedPVal = StatisticalUtils::dunnSidak(
					pValue, m_numTripletsForStatCorrection);
				if (correctedPVal < UserSettings::instance().rejectThreshold) {
					threadData.numsRecombinantTriplets[i]++;
					if (child->getRecombinantType() == Sequence::RecombinantType::NotRec) {
						child->setRecombinantType(
							Sequence::RecombinantType::Short);
					}
					threadData.tripletPools[i].saveTriplet(child, triplet);

				}
				else {
					threadData.tripletPools[i].freeTriplet(triplet);
				}
			}
		}
	}
	// TODO: add entry to the technical log file about finishing
	threadData.guard.lock();
	threadData.containerIsLocked[i] = false;
	threadData.processedChunks++;
	threadData.guard.unlock();
}

void RecombinantDetector::displayResult() {
	size_t numRecombinant = 0;
	size_t numLongRec = 0;

	for (const auto& child : m_alignment.getActiveChildren()) {
		if (child->getRecombinantType() != Sequence::RecombinantType::NotRec) {
			numRecombinant++;
		}
		if (child->getRecombinantType() == Sequence::RecombinantType::Long) {
			numLongRec++;
			if (m_fileLongRecombinants != nullptr) {
				m_fileLongRecombinants->writeLine(child->name());
			}
		}
	}

	App::instance()
		<< "Number of triples tested :              "
		<< m_alignmentDescriptor.getTripletCounts()->active << "\n"
		<< "Number of p-values computed exactly :   " << m_numComputedExactly
		<< "\n"
		<< "Number of p-values approximated (HS) :  " << m_numApproximated
		<< "\n"
		<< "Number of p-values not computed :       " << m_numSkipped << "\n"
		<< endl
		<< "Number of recombinant triplets :                               \t"
		<< m_numRecombinantTriplets << "\n"
		<< "Number of distinct recombinant sequences :                     \t"
		<< numRecombinant << "\n";

	if (!UserSettings::instance().calculateNoBreakpoints) {
		App::instance()
			<< "Number of distinct recombinant sequences at least "
			<< UserSettings::instance().minLongRecombinationThreshold << "nt long : \t" << numLongRec
			<< "\n"
			<< "Longest of short recombinant segments :                        \t"
			<< m_tripletPool.getLongestMinRecLength() << "nt\n";
	}

	if (m_tryToGetDataFromSequenceName) {
		App::instance()
			<< "Number of skipped triplets due to discrepancies between the dates of the parent and the child:    "
			<< m_numTripletsSkippedByTime;
	}
	App::instance().showOutput(true);

	App::instance() << App::SEPARATOR << endl;
	App::instance().showLog(true);

	char formatedPVal[20];
	sprintf(formatedPVal,
		"%1.3e",
		static_cast<double> (StatisticalUtils::dunnSidak(m_minPVal)));
	App::instance()
		<< "Rejection of the null hypothesis of clonal evolution at p = "
		<< StatisticalUtils::dunnSidak(m_minPVal)
		<< "\n"
		<< "                                                        p = "
		<< formatedPVal << "\n"
		<< "                                            Uncorrected p = "
		<< m_minPVal << "\n"
		<< "                                            Bonferroni  p = "
		<< StatisticalUtils::bonferroni(m_minPVal)
		<< "\n";
	App::instance().showOutput(true);
}