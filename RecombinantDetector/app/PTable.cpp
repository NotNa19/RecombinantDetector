
#include "PTable.h"
#include "PTableFile.h"
#include "UserSettings.h"
#include "App.h"

////////////////////////////////////////////////////////////////////////////////
//  STATIC CONSTANTS
////////////////////////////////////////////////////////////////////////////////

const double PTable::NaPVAL = -1.0;

/* 1 MB = 1048576 Byte - use double to be easy in calculating. */
const double PTable::BYTE_IN_MB = 1048576.0;
const double PTable::FLOAT_SIZE = static_cast<double> (sizeof(float));

////////////////////////////////////////////////////////////////////////////////

void PTable::clearTable() {
	if (indexArray) {
		delete[] indexArray;
		indexArray = nullptr;
	}

	if (table) {
		delete[] table;
		table = nullptr;
	}

	if (ykTableForLastK) {
		delete ykTableForLastK;
		ykTableForLastK = nullptr;
	}
}

long PTable::estimateMemNeededInMB(const long& newMSize,
	const long& newNSize,
	const long& newKSize) {
	long numOfStoredPVals = 0.0;
	/* Special cases will not be stored:
	 *      m < 1
	 *      n < 1      n < k
	 *      k < 2      k < n - m + 1
	 */
	for (long m = 1; m <= newMSize; m++) {
		for (long n = 1; n <= newNSize; n++) {
			auto minK = (n > m + 1) ? n - m + 1 : 2;
			auto maxK = (n < newKSize) ? n : newKSize;
			auto numOfAcceptedK = (maxK >= minK) ? maxK - minK + 1 : 0;
			numOfStoredPVals += numOfAcceptedK;
		}
	}

	auto memUsed = static_cast<double>(numOfStoredPVals) * FLOAT_SIZE / BYTE_IN_MB;
	if (memUsed <= 0.0) {
		memUsed = 1.0;
	}
	return static_cast<long> (ceil(memUsed));
}

long PTable::initIndexArray() {
	assert(indexArray == nullptr);

	indexArray = new long[mSize * nSize];

	long totalNumOfStoredPVals = 0;

	/* Special cases will not be stored:
	 *      m < 1
	 *      n < 1      n < k
	 *      k < 2      k < n - m + 1
	 */
	for (long m = 1; m <= mSize; m++) {
		for (long n = 1; n <= nSize; n++) {
			indexArray[(m - 1) * nSize + n - 1] = totalNumOfStoredPVals;

			long minK = (n - m + 1 > 2) ? n - m + 1 : 2;
			long maxK = (n < kSize) ? n : kSize;
			long numOfAcceptedK = (maxK >= minK) ? maxK - minK + 1 : 0;
			totalNumOfStoredPVals += numOfAcceptedK;
		}
	}

	return totalNumOfStoredPVals;
}

void PTable::initialize(const long& newMSize,
	const long& newNSize,
	const long& newKSize) {
	clearTable();

	mSize = newMSize;
	nSize = newNSize;
	kSize = newKSize;

	assert(mSize > 0 && nSize > 0 && kSize > 0);
	numStoredVals = initIndexArray();

	table = new(nothrow) float[numStoredVals];

	if (!table) {
		App::instance() << "Unable to allocate savedTriplets for the P-value table.\n";
		App::instance().showError(true, true);
	}
}

void PTable::generateTable(const long& newMSize,
	const long& newNSize,
	const long& newKSize) {
	long memNeeded = estimateMemNeededInMB(newMSize, newNSize, newKSize);

	App::instance()
		<< "The program needs ~" << memNeeded * 2
		<< "MB of RAM to generate the P-value table.\n"
		<< "The real size of the table is ~"
		<< memNeeded << "MB.";

	initialize(newMSize, newNSize, newKSize);

	ykTableForLastK = new YkTable(mSize, nSize, kSize);

	time_t lastTime = time(nullptr) - UserSettings::instance().UpdateMonitorInSec - 1;
	App::instance().initCounter("Generating P-value table", 2, kSize);

	/* k and n will be started from 2 since 0 and 1 are special cases */
	for (long k = 2; k <= kSize; k++) {
		time_t currentTime = time(nullptr);
		if (currentTime - lastTime >= UserSettings::instance().UpdateMonitorInSec) {
			App::instance() << "  -   Elapsed time :  "
				<< App::instance().getElapsedTime();
			App::instance().count(k);
			lastTime = currentTime;
		}

		ykTableForLastK->generateTable(k - 1);

		for (long m = 1; m <= mSize; m++) {

			/* Since n < k and n >= m + k are special cases */
			long maxN = (m + k < nSize) ? m + k : nSize;
			for (long n = k; n <= maxN; n++) {
				calculatePVal(m, n, k);
			}
		}

	}
	App::instance().finishCounting();

	delete ykTableForLastK;
	ykTableForLastK = nullptr;
}

void PTable::calculatePVal(const long& nUpSteps,
	const long& nDownSteps,
	const long& m_maxDescent) {
	assert(nUpSteps >= 0 && nDownSteps >= 0 && m_maxDescent >= 0);
	if (m_maxDescent > nDownSteps
		|| nUpSteps == 0 /* maxDescent <= nDownSteps */
		|| nDownSteps == 0 /* maxDescent == 0 */
		|| m_maxDescent == 0 || m_maxDescent == 1 /* nDownSteps >= 1 */
		|| nDownSteps - nUpSteps >= m_maxDescent) {
		/* These cases are not necessary to be calculated */
		return;
	}

	assert(nUpSteps <= mSize && nDownSteps <= nSize && m_maxDescent <= kSize && nUpSteps >= 1
		&& nDownSteps >= 1 && m_maxDescent >= 2);

	long index = get1DIndex(nUpSteps, nDownSteps, m_maxDescent);
	assert(index >= 0 && index < numStoredVals);

	auto fM = static_cast<float> (nUpSteps);
	auto fN = static_cast<float> (nDownSteps);

	float pVal =
		(fM * getExactPValue(nUpSteps - 1, nDownSteps, m_maxDescent)
			+ fN * getExactPValue(nUpSteps, nDownSteps - 1, m_maxDescent)
			+ fN * ykTableForLastK->getYValue(nUpSteps, nDownSteps - 1, m_maxDescent - 1,
				m_maxDescent - 1)
			) / (fM + fN);
	table[index] = pVal;
}

bool PTable::saveToFile(const string& fileName) const {
	PTableFile pTableFile(fileName);
	return pTableFile.save(*this);
}


////////////////////////////////////////////////////////////////////////////////
//
//       Yk-Table class (private class)
//
////////////////////////////////////////////////////////////////////////////////

PTable::YkTable::YkTable(long newMSize, long newNSize, long newJSize) {
	mSize = newMSize;
	nSize = newNSize;
	jSize = newJSize;

	currentK = -1;

	lastYkk = new float[mSize * nSize];
	numStoredVals = initIndexArray();
	table = new(nothrow) float[numStoredVals];

	if (!table) {
		App::instance()
			<< "Unable to allocate savedTriplets for Y-value table.\n"
			<< "Try creating a smaller table.\n";
		App::instance().showError(true, true);
	}

	/* Initialise to reduce later calculation */
	for (long i = 0; i < numStoredVals; i++) {
		table[i] = 0.0f;
	}
}

PTable::YkTable::~YkTable() {
	if (indexArray) {
		delete[] indexArray;
		indexArray = nullptr;
	}

	if (lastYkk) {
		delete[] lastYkk;
		lastYkk = nullptr;
	}

	if (table) {
		delete[] table;
		table = nullptr;
	}
}

long PTable::YkTable::initIndexArray() {
	indexArray = new long[mSize * nSize];

	long totalNumOfValues = 0;

	/* m, n will begin from 1 since the cases where m, n = 0 are not
	 * needed to be stored */
	for (long m = 1; m <= mSize; m++) {
		for (long n = 1; n <= nSize; n++) {
			indexArray[(m - 1) * nSize + n - 1] = totalNumOfValues;

			auto minJ = (n - m > 0) ? n - m : 0;
			auto maxJ = (n < jSize) ? n : jSize;
			auto numOfAcceptedJValues = (maxJ >= minJ) ? maxJ - minJ + 1 : 0;
			totalNumOfValues += numOfAcceptedJValues;
		}
	}

	return totalNumOfValues;
}

void PTable::YkTable::generateTable(const long& k) {
	assert(k >= 1 && k <= jSize);

	if (currentK < 0 || k == 1) {
		/* Make sure this table is generated from k = 1 */
		assert(k == 1);

		/* Initialise lastYkk */
		for (long m = 1; m <= mSize; m++) {
			for (long n = 1; n <= nSize; n++) {
				/* Since k-1 = 0 and m, n > 0, all Y[m, n, k-1, k-1] = 0 */
				lastYkk[(m - 1) * nSize + n - 1] = 0.0f;
			}
		}

	}
	else {
		/* Make sure that the new k == currentK + 1 because the layer of k
		 * cannot be generated without the layer of k-1 */
		assert(k == currentK + 1);

		/* Update lastYkk */
		for (long m = 1; m <= mSize; m++) {
			auto minN = k - 1; // k >= 2
			auto maxN = k - 1 + m;
			if (maxN > nSize) maxN = nSize;

			/* Since k > n and k+m < n are special cases */
			for (auto n = minN; n <= maxN; n++) {
				lastYkk[(m - 1) * nSize + n - 1] = getYValue(m, n, k - 1, k - 1);
			}
		}
	}

	currentK = k;

	/* Generate new layer */
	for (long m = 1; m <= mSize; m++) {
		auto maxN = currentK + m;
		if (maxN > nSize) maxN = nSize;

		/* Since k > n and k+m < n are special cases */
		for (auto n = currentK; n <= maxN; n++) {
			auto minJ = (n - m > 0) ? n - m : 0;
			auto maxJ = (n < jSize) ? n : jSize;

			for (long j = minJ; j <= maxJ; j++) {
				long ykIndex = get1DIndex(m, n, j);
				assert(ykIndex >= 0 && ykIndex < numStoredVals);

				auto fM = static_cast<float> (m);
				auto fN = static_cast<float> (n);

				if (j == 0) {
					table[ykIndex] = (fM / (fM + fN))
						* (getYValue(m - 1, n, k, 1) + getYValue(m - 1, n, k, 0));

				}
				else if (j == currentK) {
					float Y_m_nPre_kPre_kPre = 0.0f; // Y[m, n-1, k-1, k-1]
					if (n == 1) {
						if (currentK == 1) {
							/* n-1 = k-1 = j-1 = 0 */
							Y_m_nPre_kPre_kPre = 1.0f;
						}
					}
					else {
						Y_m_nPre_kPre_kPre = lastYkk[(m - 1) * nSize + (n - 1) - 1];
					}

					table[ykIndex] = (fN / (fM + fN))
						* (Y_m_nPre_kPre_kPre + getYValue(m, n - 1, k, j - 1));

				}
				else {
					table[ykIndex] = (fM * getYValue(m - 1, n, k, j + 1)
						+ fN * getYValue(m, n - 1, k, j - 1)
						) / (fM + fN);
				}


			}
		}
	}
}

