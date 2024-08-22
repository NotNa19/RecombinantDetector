#ifndef RUN_H
#define	RUN_H

#include <cassert>
#include <string>
#include <vector>
#include <limits>

#include "../App.h"
#include "../FastaReader.h"
#include "../TextFile.h"
#include "../PTableFile.h"

#include "../../core/Alignment.h"
#include "../../utils/StringUtils.h"

class Run {
public:

	enum class Mode {
		GeneratePTable, RecombinantDetection
	};

	Run(const Run& orig) = delete;

	Run& operator=(const Run& rhs) = delete;

	virtual ~Run();

	static Run* getRun(int argc, char** argv);

	virtual void parseCmdLine();

	virtual void perform();

	virtual void initLogFile() const;

	virtual bool isLogFileSupported() const = 0;

	virtual Mode getMode() const = 0;

protected:
	Run() = delete;

	explicit Run(int argc, char** argv);

	// Delete the first numDelete arguments
	virtual void deleteCmdLineArgs(int numDelete);

	virtual int getRunArgsNum() const;
	virtual void addPValIntoHistogram(double pValue);
	virtual void savePValHistogram(const char& separator);
	virtual void loadPTable(PTableFile* pTableFile);

	Alignment m_alignment;

	int m_argCount{};

	std::vector<std::string> m_argVector;

private:
	std::vector<size_t> m_pValsHistogram;

	std::string m_pValHistogramFileName;

};

#endif	/* RUN_H */
