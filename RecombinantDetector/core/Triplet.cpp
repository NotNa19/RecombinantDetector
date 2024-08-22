#include "Triplet.h"

#include "../app/PTable.h"
#include "../app/TextFile.h"
#include "../app/App.h"
#include "../app/UserSettings.h"
#include "../utils/numeric_types.h"

size_t Triplet::m_longRecombinantThreshold = 100;
bool Triplet::m_isApproximatePValAccepted = true;

void Triplet::setLongRecombinantThreshold(const size_t& newThreshold) {
	Triplet::m_longRecombinantThreshold = newThreshold;
}

void Triplet::setAcceptApproxPVal(const bool& acceptApproxPVal) {
	Triplet::m_isApproximatePValAccepted = acceptApproxPVal;
}

std::shared_ptr<Triplet> Triplet::create() {
	return std::make_shared<Triplet>();
}

Triplet::Triplet()
	: m_child(nullptr), m_dad(nullptr), m_mum(nullptr),
	m_leftBreakPoints(), m_rightBreakPoints(),
	m_upStep(0), m_downStep(0), m_maxDescent(0), m_minRecombinantLength(0),
	m_exactPValue(Double::NOT_SET), m_approxPValue(Double::NOT_SET),
	m_randomWalkHeights(), m_mostRecentMaxHeights() {
}

void Triplet::reassign(const SequencePtr& newChild,
	const SequencePtr& newDad,
	const SequencePtr& newMum) {
	m_child = newChild;
	m_dad = newDad;
	m_mum = newMum;

	m_minRecombinantLength = 0;

	m_leftBreakPoints.clear();
	m_rightBreakPoints.clear();
	m_breakPointsPairs.clear();

	updateSteps();
	computePValues();
}

void Triplet::updateSteps() {
	size_t activeSeqLen = m_child->activeLength();

	m_upStep = 0;
	m_downStep = 0;
	m_maxDescent = 0;

	m_randomWalkHeights.resize(activeSeqLen + 1);
	m_mostRecentMaxHeights.resize(activeSeqLen + 1);
	m_randomWalkHeights[0] = 0;
	m_mostRecentMaxHeights[0] = 0;

	for (size_t activeNuIdx = 0; activeNuIdx < activeSeqLen; activeNuIdx++) {
		auto dadNu = m_dad->getActiveNuc(activeNuIdx);
		auto mumNu = m_mum->getActiveNuc(activeNuIdx);
		auto childNu = m_child->getActiveNuc(activeNuIdx);

		auto currentHeight = m_randomWalkHeights[activeNuIdx];
		if (dadNu != Nucleotide::Gap && mumNu != Nucleotide::Gap && childNu != Nucleotide::Gap) {
			if (dadNu == childNu && mumNu != childNu) {
				m_upStep++;
				currentHeight++;

			}
			else if (dadNu != childNu && mumNu == childNu) {
				m_downStep++;
				currentHeight--;
			}
		}
		m_randomWalkHeights[activeNuIdx + 1] = currentHeight;

		if (currentHeight > m_mostRecentMaxHeights[activeNuIdx]) {
			m_mostRecentMaxHeights[activeNuIdx + 1] = currentHeight;
		}
		else {
			m_mostRecentMaxHeights[activeNuIdx + 1] = m_mostRecentMaxHeights[activeNuIdx];
		}

		auto maxDescentToThisPoint = m_mostRecentMaxHeights[activeNuIdx + 1] - currentHeight;
		if (m_maxDescent < maxDescentToThisPoint) {
			m_maxDescent = maxDescentToThisPoint;
		}
	}
}

void Triplet::computePValues() {
	m_exactPValue = Double::NOT_SET;
	m_approxPValue = Double::NOT_SET;


	if (PTable::instance().canCalculateExact(m_upStep, m_downStep, m_maxDescent)) {
		m_exactPValue = PTable::instance().getExactPValue(m_upStep, m_downStep, m_maxDescent);
	}
	else if (m_isApproximatePValAccepted) {
		m_approxPValue = PTable::approxPValue(m_upStep,
			m_downStep,
			m_maxDescent);
	}
}

double Triplet::getPValue() const {
	if (hasExactPVal()) {
		return m_exactPValue;
	}
	else {
		return m_approxPValue;
	}
}

void Triplet::seekBreakPoints() {
	if (!m_leftBreakPoints.empty() && !m_rightBreakPoints.empty()) {
		return; // break-points are already calculated
	}

	auto randomWalkLength = m_randomWalkHeights.size();
	auto randomWalkPos = randomWalkLength;
	long heightOfLeftBp = Long::NOT_SET;
	do {
		randomWalkPos--;
		if (m_mostRecentMaxHeights[randomWalkPos] == m_randomWalkHeights[randomWalkPos] + m_maxDescent) {
			// This position is a right breakpoint
			auto breakPoint = buildBpFromRight(randomWalkPos, true);
			m_rightBreakPoints.push_back(breakPoint);
			heightOfLeftBp = m_mostRecentMaxHeights[randomWalkPos];

		}
		else if (m_randomWalkHeights[randomWalkPos] == heightOfLeftBp
			&& !m_rightBreakPoints.empty()) {
			// This position is a left breakpoint
			auto breakPoint = buildBpFromRight(randomWalkPos, false);
			m_leftBreakPoints.push_back(breakPoint);
		}
	} while (randomWalkPos > 0);
}

BreakPointPtr Triplet::buildBpFromRight(size_t& randomWalkPos,
	const bool& buildingRightBp) {
	auto rightActiveNuIdx = randomWalkPos;
	while (randomWalkPos > 0
		&& m_randomWalkHeights[randomWalkPos - 1] == m_randomWalkHeights[randomWalkPos]) {
		randomWalkPos--;
	}
	auto leftActiveNuIdx = randomWalkPos;

	if (buildingRightBp) {
		// The active position belongs to the dad (P) sequence
		// -> left-shift it to make the position point to the end of the mum (Q) segment
		leftActiveNuIdx--;
		rightActiveNuIdx--;
	}

	size_t leftOrigNuPos = 0;
	if (leftActiveNuIdx > 0) {
		leftOrigNuPos = m_child->getOriginalPositionOfActiveNuc(leftActiveNuIdx - 1) + 1;
	}
	size_t rightOrigNuPos = m_child->fullLength() - 1;
	if (rightActiveNuIdx < m_child->activeLength() - 1) {
		rightOrigNuPos = m_child->getOriginalPositionOfActiveNuc(rightActiveNuIdx + 1) - 1;
	}

	if (buildingRightBp) {
		// The right break point need to be right-shift by 1 position
		// to cover the last nucleotide of the mum (Q) segment.
		leftOrigNuPos++;
		rightOrigNuPos++;
	}

	return BreakPoint::create(leftOrigNuPos,
		rightOrigNuPos,
		m_randomWalkHeights[randomWalkPos]);
}

void Triplet::seekBreakPointPairs() {
	if (!m_breakPointsPairs.empty()) {
		return; // break-point pairs are already calculated
	}

	seekBreakPoints();
	m_minRecombinantLength = ULong::MAX;

	// Loop through the breakpoint vectors from right to left since the leftmost breakpoints
	// are stored at the right-end of the vectors.
	for (auto leftBpIter = m_leftBreakPoints.rbegin();
		leftBpIter != m_leftBreakPoints.rend(); ++leftBpIter) {
		for (auto rightBpIter = m_rightBreakPoints.rbegin();
			rightBpIter != m_rightBreakPoints.rend(); ++rightBpIter) {

			if ((*leftBpIter)->rightBound < (*rightBpIter)->leftBound) {
				if ((*leftBpIter)->height - (*rightBpIter)->height == m_maxDescent) {
					m_breakPointsPairs.emplace_back(*leftBpIter, *rightBpIter);

					auto mumSegmentLength = countNonGappedSites(
						(*leftBpIter)->rightBound,
						(*rightBpIter)->leftBound);
					if (m_minRecombinantLength > mumSegmentLength) {
						m_minRecombinantLength = mumSegmentLength;
					}

					auto dadSegmentLength =
						countNonGappedSites(0,
							(*leftBpIter)->leftBound)
						+ countNonGappedSites((*rightBpIter)->rightBound,
							m_child->fullLength());
					if (m_minRecombinantLength > dadSegmentLength) {
						m_minRecombinantLength = dadSegmentLength;
					}

				}
				else {
					break;
				}
			}
		}
	}

	if (m_minRecombinantLength >= Triplet::m_longRecombinantThreshold
		&& m_child->getRecombinantType() == Sequence::RecombinantType::Short) {
		m_child->setRecombinantType(Sequence::RecombinantType::Long);
	}
}

size_t Triplet::countNonGappedSites(const size_t& leftOrigNuPos,
	const size_t& rightOrigNuPos) const {
	size_t nNonGappedSites = 0;
	for (auto origNuPos = leftOrigNuPos; origNuPos < rightOrigNuPos; origNuPos++) {
		if (m_child->m_nucleotides[origNuPos] != Nucleotide::Gap
			&& m_dad->m_nucleotides[origNuPos] != Nucleotide::Gap
			&& m_mum->m_nucleotides[origNuPos] != Nucleotide::Gap) {
			nNonGappedSites++;
		}
	}
	return nNonGappedSites;
}

std::string Triplet::info() const {
	std::stringstream sstream;
	sstream << m_dad->name() << "\t"
		<< m_mum->name() << "\t"
		<< m_child->name() << "\t"
		<< m_upStep << "\t"
		<< m_downStep << "\t"
		<< m_maxDescent;
	return sstream.str();
}

bool Triplet::hasExactPVal() const {
	return (0 <= m_exactPValue && m_exactPValue <= 1.0);
}

bool Triplet::hasPVal() const {
	return (isSet(m_exactPValue) || isSet(m_approxPValue));
}

bool Triplet::breakPointsComputed() const {
	return !m_breakPointsPairs.empty();
}

bool Triplet::statisticallyBetter(const Triplet& another) const {
	if (this->hasPVal() && !another.hasPVal()) {
		return true;
	}
	else if (this->hasPVal() && another.hasPVal()) {
		if ((this->getPValue() < another.getPValue())
			|| (this->getPValue() == another.getPValue()
				&& this->m_minRecombinantLength > another.m_minRecombinantLength)) {
			return true;
		}
	}
	return false;
}

std::string Triplet::toString(const std::string& infoSeparator) const {
	std::string tripletInfo;
	char stringBuffer[10000];

	auto pValue = getPValue();
	auto dunnSidak = StatisticalUtils::dunnSidak(pValue);

	auto separatorCStr = infoSeparator.c_str();
	if (UserSettings::instance().simplifiedOutput){
		sprintf(stringBuffer,
			"%s%s%s%s%s%s%2.4f%s%Le",
			m_dad->name().c_str(), separatorCStr,
			m_mum->name().c_str(), separatorCStr,
			m_child->name().c_str(), separatorCStr,
			log10(pValue), separatorCStr,
			dunnSidak
		);
	}
	else{
		sprintf(stringBuffer,
			"%s%s%s%s%s%s%lu%s%lu%s%lu%s%1.12f%s%d%s%2.4f%s%1.5Lf%s%Le",
			m_dad->name().c_str(), separatorCStr,
			m_mum->name().c_str(), separatorCStr,
			m_child->name().c_str(), separatorCStr,
			m_upStep, separatorCStr,
			m_downStep, separatorCStr,
			m_maxDescent, separatorCStr,
			pValue, separatorCStr,
			(!hasExactPVal()), separatorCStr,
			log10(pValue), separatorCStr,
			dunnSidak, separatorCStr,
			dunnSidak
		);
	}

	tripletInfo = std::string(stringBuffer);

	if (breakPointsComputed()) {
		sprintf(stringBuffer, "%s%lu", separatorCStr, m_minRecombinantLength);
		tripletInfo += std::string(stringBuffer);
		for (const auto& bpPair : m_breakPointsPairs) {
			tripletInfo += infoSeparator + bpPair.toString();
		}
	}

	return tripletInfo;
}
