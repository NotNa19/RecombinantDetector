#include "App.h"
#include "UserSettings.h"
#include "PTableFile.h"

#include "../utils/Utils.h"

const std::string App::SEPARATOR
= "--------------------------------------------------------------------------------";
const std::string App::BOLD_SEPARATOR
= "--------------------------------------------------------------------------------";

const std::string App::DEFAULT_INDENT = "  ";
const std::string App::ERROR_STR = "ERROR: ";

const std::string App::ERROR_FORMAT = "31";
const std::string App::WARNING_FORMAT = "33";
const std::string App::OUTPUT_FORMAT = "1;32";

void App::initLogFile(std::string logFileName) {
	TextFile* newLogFile = new TextFile(logFileName);

	if (newLogFile->exists()) {
		m_logFile = newLogFile;
		m_logFile->removeFile();
		clear();

	}
	else {
		m_logFile = newLogFile;
	}

	if (m_logFile != nullptr) {
		m_logFile->openToWrite();
		m_logFile->writeLine(BOLD_SEPARATOR);
		m_logFile->writeLine(BOLD_SEPARATOR + "\n");

		if (m_runCommand.length() > 0) {
			m_logFile->writeLine("COMMAND:");
			m_logFile->writeLine(DEFAULT_INDENT + m_runCommand + "\n");
		}
	}
}

const std::string App::align(std::string firstIndent) {
	std::string alignedStr = "";
	std::string nextIndent = "";

	for (size_t i = 0; i < firstIndent.length(); i++) {
		if (firstIndent[i] == '\n' || firstIndent[i] == '\r') {
			nextIndent = "";
		}
		else if (firstIndent[i] == '\t') {
			nextIndent += "\t";
		}
		else {
			nextIndent += " ";
		}
	}

	stringstream tmpStream;
	tmpStream.str(this->str());
	std::string newLine = "";
	int lineCount = 0;

	do {
		std::getline(tmpStream, newLine);

		if (lineCount > 0) {
			alignedStr += "\n";
		}

		if (newLine.length() > 0) {
			if (lineCount <= 0) {
				alignedStr += firstIndent + newLine;
			}
			else {
				alignedStr += nextIndent + newLine;
			}
		}

		lineCount++;
	} while (!tmpStream.eof() && !tmpStream.fail());

	return alignedStr;
}

void App::startProgram(const std::string& startMessage) {
	std::string settingsPath = "../settings/user_settings.rec";
	UserSettings::instance().readSettings(settingsPath);
	clear();
	logSettings();
	switch (m_mode) {
	case Mode::CONSOLE:
		(*this) << "Program started" << endl
			<< startMessage << endl;
		break;

	case Mode::SILENT:
		break;
	}

	showLog(false);
	clear();
	std::cout.flush();
}

void App::logSettings() {
	auto& settings = UserSettings::instance();
	(*this) << "The program is running with the following settings:" << endl;
	(*this) << "Use header data enabled: " << settings.tryToGetDataFromSequenceName << endl;
	(*this) << "Time threshold between parents and child: " << settings.timeThresholdBetweenParentsAndChild << endl;
	(*this) << "Multi file mode: " << settings.multiFileMode << endl;
	(*this) << "Threads count: " << settings.threadsCount << endl;
	(*this) << "Use separate child file enabled: " << settings.useSeparateChildFile << endl;
	(*this) << "Separate child file path: " << settings.separateChildFilePath << endl;
	(*this) << "Sequences to read limit enabled: " << settings.sequencesToReadLimitEnabled << endl;
	(*this) << "Sequences to read limit: " << settings.sequencesToReadLimit << endl;
	(*this) << "P-table file path: " << settings.pTableFilePath << endl;
	showLog(true);
}

void App::showPTableLink() {
	std::string lastUsedPTableFile = "";

	try {
		// Quickly validate the file
		fstream pTableFile(lastUsedPTableFile.c_str(), ios::in | ios::binary);
		if (!pTableFile.is_open() || pTableFile.fail() || pTableFile.eof()) {
			// File not valid
			lastUsedPTableFile = "";
		}
		pTableFile.close();

	}
	catch (...) {
		lastUsedPTableFile = "";
	}

	clear();

	switch (m_mode) {
	case Mode::CONSOLE:
		if (lastUsedPTableFile.length() > 0) {
			(*this) << "The current p-value table is located at: \""
				<< lastUsedPTableFile << "\""
				<< endl;
		}
		else {
			(*this) << "There is no p-value table" << endl;
		}
		std::cout << this->align(DEFAULT_INDENT) << endl;
		break;
	case Mode::SILENT:
		// Show nothing
		break;
	}

	clear();
	std::cout.flush();
}

void App::throwExitSignal(int status) {
	try {
		clear();

		switch (m_mode) {
		case Mode::CONSOLE:
			(*this) << BOLD_SEPARATOR << endl;
			std::cout << this->align(DEFAULT_INDENT) << endl << endl;
			break;
		case Mode::SILENT:
			// Show nothing
			break;
		}

		if (m_logFile != nullptr) {
			time_t currentTime;
			struct tm* timeInfo;
			std::string timeStr;
			time(&currentTime);
			timeInfo = localtime(&currentTime);

			timeStr = StringUtils::formatInt(timeInfo->tm_year + 1900, 4) + "/"
				+ StringUtils::formatInt(timeInfo->tm_mon + 1, 2) + "/"
				+ StringUtils::formatInt(timeInfo->tm_mday, 2)
				+ " "
				+ StringUtils::formatInt(timeInfo->tm_hour, 2) + ":"
				+ StringUtils::formatInt(timeInfo->tm_min, 2) + ":"
				+ StringUtils::formatInt(timeInfo->tm_sec, 2);

			m_logFile->writeLine(BOLD_SEPARATOR + "\n\n");
			m_logFile->writeLine(
				"                                            ╔══════════════════════════════════╗");
			m_logFile->writeLine(
				"                                            ║ Log saved at " + timeStr + " ║");
			m_logFile->writeLine(
				"                                            ╚══════════════════════════════════╝");
			m_logFile->writeLine();
			m_logFile->close();
		}

		clear();
		std::cout.flush();


	}
	catch (...) {
		/* Do nothing.
		 * The reason for using try-catch is about to make sure that the
		 * exitSignal will be throw.
		 */
	}


	if (status == 0) {
		throw SUCCESS;
	}
	else {
		throw FAIL;
	}
}

void App::showOutput(bool autoEndl) {
	assert(!m_isCounting);

	switch (m_mode) {
	case Mode::CONSOLE:
	case Mode::SILENT:
		setOutputStreamFormat(OUTPUT_FORMAT);
		std::cout << this->align(DEFAULT_INDENT);
		if (autoEndl) {
			std::cout << endl;
		}
		cleanOutputStreamFormat();
		break;
	}

	if (m_logFile != nullptr) {
		if (autoEndl)
			m_logFile->writeLine(this->str());
		else
			m_logFile->write(this->str());
	}

	clear();
	std::cout.flush();
}

void App::showLog(bool autoEndl) {
	assert(!m_isCounting);

	if (m_isLogStreamEnabled) {
		switch (m_mode) {
		case Mode::CONSOLE:
		case Mode::SILENT:
			std::cout << this->align(DEFAULT_INDENT);
			if (autoEndl) {
				std::cout << endl;
			}
			break;
		}
	}

	if (m_logFile != nullptr && m_isLogFileEnabled) {
		if (autoEndl)
			m_logFile->writeLine(this->str());
		else
			m_logFile->write(this->str());
	}

	clear();
	std::cout.flush();
}

void App::showError(bool autoEndl, bool exitAfterShowing) {
	if (m_isCounting) {
		count(m_counter.currentValue, true);
	}

	switch (m_mode) {
	case Mode::CONSOLE: // fall through
	case Mode::SILENT:
		setErrorStreamFormat(ERROR_FORMAT);
		cerr << this->align(DEFAULT_INDENT + ERROR_STR);
		if (autoEndl) {
			cerr << endl;
		}
		cleanErrorStreamFormat();
		break;
	}

	if (m_logFile != nullptr) {
		if (autoEndl)
			m_logFile->writeLine(this->align(ERROR_STR));
		else
			m_logFile->write(this->align(ERROR_STR));
	}

	clear();
	cerr.flush();

	if (exitAfterShowing) {
		throwExitSignal(1);
	}
}

void App::count(double currentValue, bool isLastValue /*= false*/) {
	assert(m_isCounting);

	m_counter.currentValue = currentValue;
	std::string message = m_counter.showValue();

	if (m_counter.processingText.length() > 0) {
		message = m_counter.processingText + " :  " + message;
	}
	if (m_counter.isPercentCounter) {
		message += "%";
	}
	if (this->str().length() > 0) {
		message += " " + this->str();
	}
	if (isLastValue) {
		message += "\n";
	}
	else {
		message += "\r";
	}
	this->str(message);


	switch (m_mode) {
	case Mode::CONSOLE:
		std::cout << this->align(DEFAULT_INDENT);
		break;
	case Mode::SILENT:
		break;
	}

	if (isLastValue) {
		m_isCounting = false;
		if (m_logFile != nullptr) {
			m_logFile->write(this->str());
		}
	}

	clear();
	std::cout.flush();
}

void App::initCounter(std::string processingText, double minValue,
	double maxValue, bool percentCounter /*= true*/) {

	m_counter.initialize(processingText, minValue, maxValue, percentCounter);
	m_isCounting = true;
}

void App::finishCounting() {
	count(m_counter.maxValue, true);
}

void App::enableLog(const bool& isLogStreamEnabled, const bool& isLogFileEnabled) {
	this->m_isLogStreamEnabled = isLogStreamEnabled;
	this->m_isLogFileEnabled = isLogFileEnabled;
}

void App::enableWarning(const bool& isWarningEnabled) {
	this->m_isWarningEnabled = isWarningEnabled;
}

const int App::Counter::DECIMAL_PLACES = 3;

void App::Counter::initialize(std::string newProcessingText, double newMinValue,
	double newMaxValue, bool percentCounter) {

	processingText = newProcessingText;
	minValue = newMinValue;
	maxValue = newMaxValue;
	isPercentCounter = percentCounter;

	currentValue = minValue;
	m_startTime = time(nullptr);
}

const std::string App::Counter::showValue() const {
	stringstream tmpStream;

	if (isPercentCounter) {
		double percentDone = (currentValue - minValue) / (maxValue - minValue);
		percentDone *= 100.0;

		tmpStream.setf(ios::fixed);
		tmpStream.setf(ios::showpoint);
		tmpStream.precision(DECIMAL_PLACES);
		tmpStream.width(4 + DECIMAL_PLACES);
		tmpStream << percentDone;

	}
	else {
		tmpStream << currentValue;
	}

	return tmpStream.str();
}

const long App::Counter::getElapsedSeconds() const {
	time_t currentTime = time(nullptr);
	return currentTime - m_startTime;
}

void App::setExecutablePath(const std::string& executablePath) {
	if (executablePath.length() > 0 && executablePath[0] != '\0') {
		this->m_executablePath = executablePath;
	}
}

void App::setExecutablePath(const char* executablePath) {
	if (executablePath != nullptr && executablePath[0] != '\0') {
		setExecutablePath(std::string(executablePath));
	}
}