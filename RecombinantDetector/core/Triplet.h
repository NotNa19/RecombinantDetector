#pragma once
#include <vector>
#include <memory>
#include <string>

#include "Sequence.h"
#include "BreakPoint.h"

class TripletPool;

class Triplet {
public:
	friend class TripletPool;

	static void setLongRecombinantThreshold(const size_t& newThreshold);

	static void setAcceptApproxPVal(const bool& acceptApproxPVal);

	static std::shared_ptr<Triplet> create();

	Triplet(const Triplet& rhs) = delete;
	Triplet(Triplet&&) = delete;
	Triplet& operator=(const Triplet&) = delete;
	Triplet& operator=(Triplet&&) = delete;

	explicit Triplet();

	void reassign(const SequencePtr& newChild, const SequencePtr& newDad, const SequencePtr& newMum);

	double getPValue() const;

	void seekBreakPointPairs();

	bool hasExactPVal() const;
	bool hasPVal() const;

	std::string info() const;

	std::string toString(const std::string& infoSeparator = ",") const;

	bool breakPointsComputed() const;
	bool statisticallyBetter(const Triplet& another) const;

private:
	void updateSteps();
	void computePValues();
	void seekBreakPoints();

	BreakPointPtr buildBpFromRight(size_t& randomWalkPos,
		const bool& buildingRightBp);

	size_t countNonGappedSites(const size_t& leftOrigNuPos,
		const size_t& rightOrigNuPos) const;

private:
	static size_t m_longRecombinantThreshold;
	static bool m_isApproximatePValAccepted;

	size_t m_minRecombinantLength;

	SequencePtr m_child;
	SequencePtr m_dad;
	SequencePtr m_mum;

	long m_upStep;
	long m_downStep;
	long m_maxDescent;

	double m_exactPValue;
	double m_approxPValue;

	std::vector<long long> m_randomWalkHeights;

	// Stores the most recent maximum height up to the current active nucleotide position
	std::vector<long long> m_mostRecentMaxHeights;

	std::vector<BreakPointPtr> m_leftBreakPoints;
	std::vector<BreakPointPtr> m_rightBreakPoints;
	std::vector<BreakPointPair> m_breakPointsPairs;
};


typedef std::shared_ptr<Triplet> TripletPtr;

