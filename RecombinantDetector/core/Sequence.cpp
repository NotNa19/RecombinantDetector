#include "Sequence.h"

#include <utility>
#include <stdexcept>

#include "../utils/StringUtils.h"
#include "../app/UserSettings.h"

std::string Sequence::preprocessInput(const std::string& dna) {
	std::string processed;

	for (char nuc : dna) {
		auto capitalised = toupper(nuc);
		switch (capitalised) {
		case 'U':
			capitalised = static_cast<char>(Nucleotide::Thymine);
			break;
		case 'A':
		case 'C':
		case 'G':
		case 'T':
		case '-':
			break;
		default:
			capitalised = static_cast<char>(Nucleotide::Gap);
		}
		processed += static_cast<char>(capitalised);
	}

	return processed;
}

std::shared_ptr<Sequence> Sequence::create(const std::string& name, const std::string& dna) {
	return std::make_shared<Sequence>(name, dna);
}

Sequence::Sequence(std::string name, const std::string& dna)
	: m_name(std::move(name)), m_activePositions(nullptr),
	m_data(UserSettings::instance().tryToGetDataFromSequenceName ? StringUtils::extractDataFromString(m_name) : -1),
	m_recombinantType(RecombinantType::NotRec) {

	auto processedDna = preprocessInput(dna);
	for (auto nuc : processedDna) {
		this->m_nucleotides.push_back(Nucleotide(nuc));
	}
}

std::string Sequence::toString() const {
	std::string sequenceStr;
	auto seqLen = activeLength();
	for (size_t activeNuIdx = 0; activeNuIdx < seqLen; activeNuIdx++) {
		sequenceStr += static_cast<char>(getActiveNuc(activeNuIdx));
	}
	return sequenceStr;
}

size_t Sequence::activeLength() const {
	if (m_activePositions) {
		return m_activePositions->size();
	}
	else {
		return m_nucleotides.size();
	}
}

size_t Sequence::fullLength() const {
	return m_nucleotides.size();
}

Sequence::~Sequence() {
	this->m_activePositions = nullptr;
}

const std::string& Sequence::name() const {
	return m_name;
}

size_t Sequence::computeDistance(const Sequence& other, const bool& ignoreGaps) const {
	auto thisLen = this->activeLength();
	auto otherLen = other.activeLength();

	if (thisLen != otherLen) {
		throw std::length_error("For distance computation required sequences with the same length.");
	}

	size_t distance = 0;
	for (size_t idx = 0; idx < thisLen; idx++) {
		auto thisNuc = this->getActiveNuc(idx);
		auto otherNuc = other.getActiveNuc(idx);
		if (ignoreGaps && (thisNuc == Nucleotide::Gap || otherNuc == Nucleotide::Gap)) {
			continue;
		}
		else if (thisNuc != otherNuc) {
			distance++;
		}
	}

	return distance;
}

bool Sequence::isSimilarWithGaps(const Sequence& other) const {
	auto thisLen = this->activeLength();
	auto otherLen = other.activeLength();

	if (thisLen != otherLen) {
		throw std::length_error("For similarity with gap detection required sequences with the same length.");
	}

	for (size_t idx = 0; idx < thisLen; idx++) {
		auto thisNuc = this->getActiveNuc(idx);
		auto otherNuc = other.getActiveNuc(idx);
		if (thisNuc != Nucleotide::Gap && thisNuc != otherNuc) {
			return false;
		}
	}
	return true;
}

size_t Sequence::countGaps() const {
	size_t gapCount = 0;

	auto seqActiveLen = this->activeLength();
	for (size_t activeNuIdx = 0; activeNuIdx < seqActiveLen; activeNuIdx++) {
		if (this->getActiveNuc(activeNuIdx) == Nucleotide::Gap) {
			gapCount++;
		}
	}

	return gapCount;
}

size_t Sequence::getOriginalPositionOfActiveNuc(const size_t idx) const {
	if (m_activePositions) {
		return (*m_activePositions)[idx];
	}
	else {
		return idx;
	}
}

const Nucleotide& Sequence::getActiveNuc(const size_t idx) const {
	return m_nucleotides[getOriginalPositionOfActiveNuc(idx)];
}

Sequence::RecombinantType Sequence::getRecombinantType() const {
	return m_recombinantType;
}

void Sequence::setRecombinantType(Sequence::RecombinantType newRecombinantType) {
	m_recombinantType = newRecombinantType;
}

bool Sequence::isOlderThan(const Sequence& other) const {
	return (other.data() - m_data) > UserSettings::instance().timeThresholdBetweenParentsAndChild;
}