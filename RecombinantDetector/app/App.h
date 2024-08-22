#pragma once
#ifndef INTERFACE_H
#define    INTERFACE_H

#include <iostream>
#include <cstdio>
#include <cassert>
#include <sstream>
#include <string>

#include "TextFile.h"

class App : public std::stringstream {
public:

	enum ExitSignal {
		SUCCESS, FAIL
	};

	static const std::string SEPARATOR;
	static const std::string BOLD_SEPARATOR;

	static const std::string DEFAULT_INDENT;

	enum class Mode {
		CONSOLE, SILENT
	};

	App(const App& orig) = delete;

	~App() override {
		delete m_logFile;
		cleanOutputStreamFormat();
		cleanErrorStreamFormat();
	}

	static App& instance() {
		static App instance;
		return instance;
	}

	void clear() {
		str("");
		std::stringstream::clear();
	};

	// Set new output m_mode.
	void setMode(Mode newMode) {
		m_mode = newMode;
	};

	// Save the command line as a string for logging.
	void storeCommand(int argc, char** argv) {
		const std::string OP_SEPARATOR = " ";

		m_runCommand = "";
		for (int i = 1; i < argc; i++) {
			if (m_runCommand.length() > 0) {
				m_runCommand += OP_SEPARATOR;
			}
			m_runCommand += argv[i];
		}
	};

	void initLogFile(std::string logFileName);

	void startProgram(const std::string& startMessage);

	// Show the path to the current associated p-table file.
	void showPTableLink();

	void throwExitSignal(int status);

	// Show output message
	void showOutput(bool autoEndl);

	// Show log message (this method will show nothing in SILENT m_mode).
	void showLog(bool autoEndl);

	// Show error (and exit if necessary).
	void showError(bool autoEndl, bool exitAfterShowing);

	// Initialise and activate the process m_counter.
	void initCounter(std::string processingText, double minValue,
		double maxValue, bool percentCounter = true);

	// Show the counting value and deactivate the m_counter if needed.
	void count(double currentValue, bool isLastValue = false);

	/**
	 * Show the current value of the m_counter. This is useful to show the m_counter
	 * for the first time.
	 */
	void count() {
		count(m_counter.currentValue, false);
	};

	// Show the last (maximum) value of the m_counter and deactivate it.
	void finishCounting();

	// Get the elapsed time as a string which is formated as "hhh:mm:ss".
	std::string getElapsedTime() const {
		char timeFormat[] = "%03d:%02d:%02d";
		char cTime[20];

		long elapsedTime = m_counter.getElapsedSeconds();
		int hour = static_cast<int> (elapsedTime / 3600);
		int minute = static_cast<int> ((elapsedTime / 60) % 60);
		int second = static_cast<int> (elapsedTime % 60);

		sprintf(cTime, timeFormat, hour, minute, second);

		std::string result(cTime);
		return result;
	}

	long getElapsedSeconds() const {
		return m_counter.getElapsedSeconds();
	}

	void enableLog(const bool& isLogStreamEnabled, const bool& isLogFileEnabled);

	void enableWarning(const bool& isWarningEnabled);

	void setExecutablePath(const std::string& executablePath);

	void setExecutablePath(const char* executablePath);
private:

	struct Counter {
	public:
		static const int DECIMAL_PLACES;

		void
			initialize(std::string processingText, double minValue, double maxValue, bool percentCounter);

		const std::string showValue() const;

		const long getElapsedSeconds() const;

		std::string processingText;
		double minValue{};
		double maxValue{};
		double currentValue{};
		bool isPercentCounter{};

	private:
		time_t m_startTime{};
	};


	static const int PRECISION = 15;

	static const std::string ERROR_FORMAT;
	static const std::string WARNING_FORMAT;
	static const std::string OUTPUT_FORMAT;

	static const std::string ERROR_STR;

	static void setOutputStreamFormat(const std::string& format) {
		std::cout << "\e[" << format << "m";
	};

	static void setErrorStreamFormat(const std::string& format) {
		std::cerr << "\e[" << format << "m";
	};

	static void cleanOutputStreamFormat() {
		std::cout << "\e[m";
	};

	static void cleanErrorStreamFormat() {
		std::cerr << "\e[m";
	};

	explicit App() {
		m_mode = App::Mode::CONSOLE;
		m_isCounting = false;

		m_runCommand = "";

		m_logFile = nullptr;

		m_isLogStreamEnabled = true;
		m_isLogFileEnabled = true;
		m_isWarningEnabled = true;

		cleanOutputStreamFormat();
		cleanErrorStreamFormat();
	}

	App& operator=(const App& rhs) {
		if (this != &rhs) {
			assert(false);
		}

		return *this;
	}

	const std::string align(std::string firstIndent);

	void logSettings();

	Mode m_mode;

	Counter m_counter;

	bool m_isCounting;

	std::string m_runCommand;

	TextFile* m_logFile;

	bool m_isLogStreamEnabled;
	bool m_isLogFileEnabled;
	bool m_isWarningEnabled;

	std::string m_executablePath;
};

#endif	/* INTERFACE_H */
