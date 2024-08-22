#include <stdexcept>

#include "Run.h"

#include "RecombinantDetector.h"
#include "PTableGenerator.h"

#include "../UserSettings.h"

#include "../../utils/numeric_types.h"
#include "../../core/AlignmentDescriptor.h"

Run::Run(int argc, char** argv)
	: m_argCount(argc), m_alignment(),
	m_pValsHistogram(UserSettings::instance().PvalHistogramSize, 0) {
	App::instance().storeCommand(argc, argv);

	m_argVector.reserve(argc);
	for (auto i = 0; i < argc; i++) {
		m_argVector.emplace_back(std::string(argv[i]));
	}
}

Run::~Run() {
}

Run* Run::getRun(int argc, char** argv) {
	if (argc < 2) {
		return nullptr;
	}

	std::string mode = argv[1];

	if (mode == "-gen-p" || mode == "-g") {
		return new PTableGenerator(argc, argv);
	}
	else if (mode == "-detect" || mode == "-d") {
		return new RecombinantDetector(argc, argv);
	}

	return nullptr;
}

void Run::deleteCmdLineArgs(int numDelete) {
	if (numDelete >= m_argCount) {
		numDelete = m_argCount - 1;
	}

	assert(numDelete >= 1);

	m_argCount -= numDelete;

	for (int i = 1; i < m_argCount; i++) {
		m_argVector[i] = m_argVector[i + numDelete];
	}
}

int Run::getRunArgsNum() const {
	if (m_argCount < 2) return -1;

	int result = 0;
	for (int i = 2; i < m_argCount; i++) {
		if (m_argVector[i][0] != '-') {
			result++;
		}
		else {
			return result;
		}
	}
	return result;
}

void Run::parseCmdLine() {
	try {
		for (int i = 1; i < m_argCount; i++) {
			if (m_argVector[i][0] == '\0') continue;

			std::string option = m_argVector[i];

			if (option[0] != '-') {
				throw option;
			}

			switch (m_argVector[i][1]) {
			case 'q':
				if (option == "-q" || option == "-quiet") {
					// Suppress all warnings during the run.
					App::instance().setMode(App::Mode::SILENT);
				}
				else {
					throw (option);
				}
				break;

			default:
				throw (option);
			}
		}

	}
	catch (std::string& unknownArg) {
		App::instance() << "Invalid command-line argument (\""
			<< unknownArg << "\")\n";
		App::instance().showError(true, true);
	}
}

void Run::initLogFile() const {
	if (!isLogFileSupported()) return;

	std::string logFileName = UserSettings::instance().DefaultLogFileName;
	App::instance().initLogFile(logFileName);
}

void Run::perform() {
	/* Prepare file names */
	m_pValHistogramFileName = UserSettings::instance().DefaultPvalHistogramFileName;

	/* Remove non-polymorphic sites if needed */
	if (!UserSettings::instance().useAllSites) {
		App::instance() << "Sequences size: " << m_alignment.fullLength() << "\n";
		m_alignment.excludeMonomorphicColumns();
		App::instance() << "Number of polymorphic sites: " << m_alignment.activeLength() << "\n";
	}

	m_alignment.getChildPool().activateSequencesBy1BasedIdxRange(
		ULong::NOT_SET, ULong::NOT_SET
	);

	/* Remove neighbour and identical sequences if needed.
	 * This must be done last since it may alter the indices of the sequence. */
	if (UserSettings::instance().removeIdenticalSequences) {
		if (UserSettings::instance().minIdenticalSequencesDistance > 0) {
			App::instance() << "Removing neighbour sequences (nt distance <= "
				<< UserSettings::instance().minIdenticalSequencesDistance << ").\n";
		}
		else {
			App::instance() << "Removing non-distinct sequences.\n";
		}
		App::instance().showLog(true);
		this->m_alignment.getParentPool().excludeNeighboringSequences(UserSettings::instance().minIdenticalSequencesDistance);
	}
}

void Run::addPValIntoHistogram(double pValue) {
	int index = static_cast<int> (-1.0 * log10(pValue));
	if (index >= UserSettings::instance().PvalHistogramSize) {
		index = UserSettings::instance().PvalHistogramSize - 1;
	}
	if (index < 0) {
		index = 0;
	}
	m_pValsHistogram[index]++;
}

void Run::savePValHistogram(const char& separator) {
	TextFile pHistFile(m_pValHistogramFileName);

	if (pHistFile.exists()) {
		pHistFile.removeFile();
	}

	try {
		AlignmentDescriptor alignmentDescriptor(this->m_alignment);
		long double nActiveTriplets = alignmentDescriptor.getTripletCounts()->active;

		pHistFile.openToWrite();

		char bufStr[200];
		for (int i = 0; i < UserSettings::instance().PvalHistogramSize; i++) {
			auto histVal = static_cast<long double> (m_pValsHistogram[i]);
			auto histPerTriplet =
				(m_pValsHistogram[i] == 0) ? 0.0L : histVal / nActiveTriplets;

			if (histPerTriplet != 0.0) {
				sprintf(bufStr,
					"%3d%c%20lu%c%1.8Lf%c%1.3Lf",
					i, separator,
					m_pValsHistogram[i], separator,
					histPerTriplet, separator,
					log10(histPerTriplet)
				);
			}
			else {
				sprintf(bufStr,
					"%3d%c%20lu%c%1.8Lf%cN/A",
					i, separator,
					m_pValsHistogram[i], separator,
					histPerTriplet, separator
				);
			}

			pHistFile.writeLine(bufStr);
		}

		pHistFile.close();

		App::instance()
			<< "The histogram of P-values has been saved in the file \""
			<< m_pValHistogramFileName << "\".\n";
		App::instance().showLog(true);

	}
	catch (...) {
		App::instance()
			<< "An error occurred during saving progress. Cannot save histogram of P-values.\n";
		App::instance().showError(true, false); // show error but not exit
	}
}

void Run::loadPTable(PTableFile* pTableFile) {
	if (pTableFile != nullptr) {

		PTableFile::ReadResult readResult = pTableFile->tryLoadInto(
			PTable::instance());
		switch (readResult) {
		case PTableFile::INVALID_FILE:
			App::instance() << "Invalid P-value table file.\n";
			App::instance().showError(true, true);
			break;

		case PTableFile::WRONG_ARCH:
			App::instance()
				<< "The P-value table file is not compatible with your system architecture.\n";
			App::instance().showError(true, true);
			break;

		case PTableFile::CANCELLED:
			App::instance() << "Loading is cancelled.\n";
			App::instance().showLog(true);
			App::instance().throwExitSignal(0);
			break;

		case PTableFile::FILE_CORRUPT:
			App::instance()
				<< "Loading fail! The P-value table file may be corrupt.\n";
			App::instance().showError(true, true);
			break;

		case PTableFile::SUCCESS:
			break;
		}

	}
	else {
		App::instance() << "Searching for P-value table file...\n";
		App::instance().showLog(true);

		PTableFile::ReadResult readResult = PTableFile::Load(PTable::instance());
		switch (readResult) {
		case PTableFile::WRONG_ARCH:    // fall through
		case PTableFile::INVALID_FILE:
			App::instance()
				<< "Cannot find any valid P-value table file.\n";
			App::instance().showError(true, true);
			break;

		case PTableFile::CANCELLED:
			App::instance() << "Loading is cancelled.\n";
			App::instance().showLog(true);
			App::instance().throwExitSignal(0);
			break;

		case PTableFile::FILE_CORRUPT:
			App::instance()
				<< "Loading fail! The P-value table file may be corrupt.\n";
			App::instance().showError(true, true);
			break;

		case PTableFile::SUCCESS:
			break;
		}
	}
}
