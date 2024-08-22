#pragma once

#include <fstream>
#include <cassert>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>

#include "../utils/StatisticalUtils.h"

using namespace std;


class PTable {
public:
	// Not a P-value. 
	static const double NaPVAL;

	~PTable() {
		clearTable();
	}

	static PTable& instance() {
		static PTable instance;
		return instance;
	}

	/**
	 * Pre-calculate the amount of RAM (in MB) that is needed to load a table of
	 * the given size.
	 */
	static long estimateMemNeededInMB(const long& newMSize,
		const long& newNSize,
		const long& newKSize);

	long getMSize() const {
		return mSize;
	}

	long getNSize() const {
		return nSize;
	}

	long getKSize() const {
		return kSize;
	}

	long getNumStoredVals() const {
		return numStoredVals;
	}

	float* getDataPtr() const {
		return table;
	}

	bool canCalculateExact(const long& nUpSteps,
		const long& nDownSteps,
		const long& m_maxDescent) const {
		if (nUpSteps < 0 || nDownSteps < 0 || m_maxDescent < 0) return false;

		// If maxDescent > nDownSteps, the P-value is always zero 
		if (m_maxDescent > nDownSteps) {
			return true;
		}

		/* In these case the p-value will be 1 */
		if (nUpSteps == 0 /* maxDescent <= nDownSteps */
			|| nDownSteps == 0 /* maxDescent == 0 */
			|| m_maxDescent == 0 || m_maxDescent == 1 /* nDownSteps >= 1 */
			|| nDownSteps - nUpSteps >= m_maxDescent) {
			return true;
		}

		return (nUpSteps <= mSize && nDownSteps <= nSize && m_maxDescent <= kSize);
	}

	double getExactPValue(const long& nUpSteps,
		const long& nDownSteps,
		const long& m_maxDescent) const {
		/* If maxDescent > nDownSteps, the P-value is always zero */
		if (m_maxDescent > nDownSteps) {
			return 0.0f;
		}

		/* In these case the p-value will be 1 */
		if (nUpSteps == 0 /* maxDescent <= nDownSteps */
			|| nDownSteps == 0 /* maxDescent == 0 */
			|| m_maxDescent == 0 || m_maxDescent == 1 /* nDownSteps >= 1 */
			|| nDownSteps - nUpSteps >= m_maxDescent) {
			return 1.0f;
		}

		assert(nUpSteps <= mSize && nDownSteps <= nSize && m_maxDescent <= kSize);

		/* Reaching here means that nUpSteps, nDownSteps >= 1 and maxDescent >= 2 */
		long index = get1DIndex(nUpSteps, nDownSteps, m_maxDescent);
		assert(index >= 0 && index < numStoredVals);

		return table[index];
	}

	static long double approxPValue(const long& nUpSteps,
		const long& nDownSteps,
		const long& m_maxDescent) {
		return StatisticalUtils::siegmundDiscreteApprox(nUpSteps, nDownSteps, m_maxDescent);
	}

	// Sets the new size and allocate the data
	void initialize(const long& newMSize,
		const long& newNSize,
		const long& newKSize);

	void generateTable(const long& newMSize,
		const long& newNSize,
		const long& newKSize);

	bool saveToFile(const string& fileName) const;


private:

	static const double BYTE_IN_MB;
	static const double FLOAT_SIZE;

	/* Disable constructor and assignment for singleton */
	PTable() {
		maxMemInMB = -1; // -1 means unlimited

		mSize = 0;
		nSize = 0;
		kSize = 0;
		numStoredVals = 0;

		indexArray = nullptr;
		table = nullptr;
		ykTableForLastK = nullptr;
	}

	PTable(const PTable& orig);

	/**
	 * Delete all P-values in the table and release savedTriplets.
	 */
	void clearTable();

	/**
	 * Initialise the indexArray and return the space (maximum # of P-values)
	 * needed to be stored in this table.
	 */
	long initIndexArray();

	/**
	 * Translate 3D-index (nUpSteps, nDownSteps, maxDescent) into 1D-index.<br>
	 * This index is used to access the P-value table.
	 */
	long get1DIndex(const long& nUpSteps,
		const long& nDownSteps,
		const long& m_maxDescent) const {
		assert(nUpSteps <= mSize && nDownSteps <= nSize && m_maxDescent <= kSize);

		/* This function will only be called when nUpSteps, nDownSteps >= 1 and maxDescent >= 2 */
		assert(nUpSteps >= 1 && nDownSteps >= 1 && m_maxDescent >= 2);

		long mnIndex = indexArray[(nUpSteps - 1) * nSize + nDownSteps - 1];

		auto minK = (nDownSteps - nUpSteps + 1 > 2) ? nDownSteps - nUpSteps + 1 : 2;
		auto kIndex = m_maxDescent - minK;

		return mnIndex + kIndex;
	}

	/**
	 * Calculate a P-value.
	 */
	void calculatePVal(const long& nUpSteps,
		const long& nDownSteps,
		const long& m_maxDescent);

	/**
	 * Maximum savedTriplets allotment for storing this table
	 */
	long maxMemInMB;

	/**
	 * Total number of P-Value stored in this table.
	 */
	long numStoredVals;

	long mSize;
	long nSize;
	long kSize;

	/**
	 * The 2D array which will be used to find the 1D-index of P[m, n, k]
	 * in the P-value table.
	 */
	long* indexArray;

	// &lt;     ~   <
	// &nbsp;   ~   SPACE
	/**
	 * The 3D table that contains P-values P[m, n, k].                      <br>
	 * Special cases will not be stored:                                    <br>
	 * &nbsp;&nbsp;&nbsp;&nbsp;     m &lt; 1                                <br>
	 * &nbsp;&nbsp;&nbsp;&nbsp;     n &lt; 1  &nbsp;&nbsp;  n &lt; k        <br>
	 * &nbsp;&nbsp;&nbsp;&nbsp;     k &lt; 2  &nbsp;&nbsp;  k &lt;= n - m   <br>
	 *                                                                      <br>
	 * The definition of P-value table:                                     <br>
	 * P[m, n, k] = sum[i:k->n] ( X[m, n, i] )                              <br>
	 *                                                                      <br>
	 * But P-value table will be calculated as the following formula:       <br>
	 *                       P[m, n, k] = m * P[m-1, n, k] / (m + n) +      <br>
	 * &nbsp;&nbsp;&nbsp;&nbsp;         + n * P[m, n-1, k] / (m + n) +      <br>
	 * &nbsp;&nbsp;&nbsp;&nbsp;         + n * Y[m, n-1, k-1, k-1] / (m + n)
	 */
	float* table;


	class YkTable;


	/**
	 * The 3D table contains Y[m, n, k-1, j] where k is a fixed value.
	 * This table is just temporarily used to create the p-value table.
	 */
	YkTable* ykTableForLastK;
};


/**
 * In this Yk-table, Yk[m, n, j] = Y[m, n, k, j]  (k is a fixed number).
 * This means Yk-table is a layer (defined by k) of Y-table. <br>
 * This table is only used temporarily to calculate the P-value table.
 *
 * @note For performance reason, all the methods of this class are NOT VIRTUAL.
 */
class PTable::YkTable {
public:
	explicit YkTable(long newMSize, long newNSize, long newJSize);

	~YkTable();

	void generateTable(const long& k);

	/**
	 * Get Y[m, n, k, j].
	 * @param m
	 * @param n
	 * @param k This is given just to make sure that the currentK of this
	 *          table is the same as the given k.
	 * @param j
	 * @return  Y[m, n, k, j]
	 */
	double getYValue(const long& m,
		const long& n,
		const long& k,
		const long& j) const {
		assert(k == currentK);
		assert(m >= 0 && n >= 0 && j >= 0);

		if (n == 0) {
			return (k == 0 && j == 0) ? 1.0f : 0.0f;

		}
		else if (m == 0) {
			return (n == k && n == j) ? 1.0f : 0.0f;

		}
		else if (k == 0 && j == 0 /*n > 0*/) {
			return 0.0f;

		}
		else if (j > k
			|| k > n || k < n - m
			|| j > n || j < n - m) {
			return 0.0f;
		}

		assert(m <= mSize && n <= nSize && j <= jSize);

		long index = get1DIndex(m, n, j);
		assert(index >= 0 && index < numStoredVals);

		return table[index];
	};


private:

	YkTable() = default;

	/* Disable assignment and copy constructor */
	YkTable(const YkTable& orig);

	YkTable operator=(const YkTable& orig);

	/**
	 * Initialise the index array and return the space (maximum # of entities)
	 * needed for the Yk-table. <br>
	 * This is used only in the constructor.
	 * @return The space (maximum # of entities) needed for the Yk-table.
	 */
	long initIndexArray();

	/**
	 * Translate 3D-index into 1D-index.<br>
	 * This index is used to access the Yk-table.
	 * @param m
	 * @param n
	 * @param j
	 * @return 1D-index
	 */
	long get1DIndex(const long& m,
		const long& n,
		const long& j) const {
		assert(m >= 1 && m <= mSize && n >= 1 && n <= nSize && j >= 0 && j <= jSize);
		long mnIndex = indexArray[(m - 1) * nSize + n - 1];

		auto minJ = (n - m > 0) ? n - m : 0;
		auto jIndex = j - minJ;

		return (mnIndex + jIndex);
	};


	long mSize;
	long nSize;
	long jSize;

	long currentK;

	/**
	 * The maximum # of entities that will be stored in this Yk-table.
	 */
	long numStoredVals;

	/**
	 * The 3D table contains Y[m, n, k, j] where k is a fixed value.
	 */
	float* table;

	/**
	 * The 2D table that contains the value of Yk'[m, n, k'] where k' = k-1 <br>
	 * This table is needed to calculate Yk[m, n, k] (j == k) since:
	 * Yk[m, n, k] = (n/(m+n)) * (Yk'[m, n-1, k'] + Yk[m, n-1, k-1])
	 */
	float* lastYkk;

	/**
	 * The 2D array which will be used to find the real index of Y[m, n, k, j]
	 * in the yTableForLastK and P[m, n, k] in the p-value table.
	 */
	long* indexArray;
};