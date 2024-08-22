#include "Alignment.h"

#include <stdexcept>

#include "Alignment.h"

#include "../utils/numeric_types.h"


Alignment::Alignment()
	: m_allelicMarkers(), m_parentPool(), m_childPool(),
	m_fullSequenceLength(0), m_locked(false), m_isSinglePool(true) {
	m_activeNucPositions = std::make_shared<std::vector<size_t>>();
}

void Alignment::addSequence(const SequencePtr& sequencePtr,
	const bool& asParent,
	const bool& asChild) {
	if (this->m_locked) {
		throw std::logic_error(
			"Sequence adding is not allowed after the alignment has been locked.");
	}
	if (!asParent && !asChild) {
		throw std::logic_error(
			"Adding sequences to the alignment -- invalid options specified.");
	}

	auto seqLen = sequencePtr->fullLength();

	if (this->m_fullSequenceLength == 0) {
		this->m_fullSequenceLength = sequencePtr->fullLength();
	}
	else if (this->m_fullSequenceLength != seqLen) {
		throw std::length_error(
			"Unmatched sequence length is found in sequence '" + sequencePtr->name()
			+ "' (expected: " + std::to_string(this->m_fullSequenceLength)
			+ ", received: " + std::to_string(seqLen) + ").");
	}

	sequencePtr->m_activePositions = this->m_activeNucPositions;
	if (asParent) {
		m_parentPool.addSequence(sequencePtr);
	}
	if (asChild) {
		m_childPool.addSequence(sequencePtr);
	}

	if (asParent != asChild) {
		this->m_isSinglePool = false;
	}
}

void Alignment::addSequences(const std::vector<SequencePtr>& sequenceList,
	const bool& asParent,
	const bool& asChild) {
	for (const auto& sequence : sequenceList) {
		this->addSequence(sequence, asParent, asChild);
	}
}

size_t Alignment::activeLength() const {
	return m_activeNucPositions->size();
}

size_t Alignment::fullLength() const {
	return m_fullSequenceLength;
}

bool Alignment::isChildParentFromSamePool() const {
	return this->m_isSinglePool;
}

void Alignment::lock() {
	if (!this->m_locked) {
		this->populatePositionVector();
		this->markAllelicStatuses();

		this->m_locked = true;
	}
}

void Alignment::populatePositionVector() {
	auto alignmentLen = this->fullLength();

	m_activeNucPositions->reserve(alignmentLen);
	for (size_t idx = 0; idx < alignmentLen; idx++) {
		m_activeNucPositions->push_back(idx);
	}
}

void Alignment::markAllelicStatuses() {
	auto allUsedSequences = getAllUsedSequences();
	auto nUsedSequences = allUsedSequences.size();
	if (nUsedSequences <= 0) {
		throw std::logic_error("The alignment does not contain any sequences; analysis halted.");
	}

	this->m_allelicMarkers.clear();
	this->m_allelicMarkers.reserve(m_fullSequenceLength);

	for (size_t nuPos = 0; nuPos < m_fullSequenceLength; nuPos++) {
		auto allelicMarker = AllelicMask::Empty;
		for (const auto& sequence : allUsedSequences) {
			switch (sequence->m_nucleotides[nuPos]) {
			case Nucleotide::Gap:
				allelicMarker |= AllelicMask::Gap;
				break;
			case Nucleotide::Adenine:
				allelicMarker |= AllelicMask::Adenine;
				break;
			case Nucleotide::Cytosine:
				allelicMarker |= AllelicMask::Cytosine;
				break;
			case Nucleotide::Guanine:
				allelicMarker |= AllelicMask::Guanine;
				break;
			case Nucleotide::Thymine:
				allelicMarker |= AllelicMask::Thymine;
				break;
			}
			if (allelicMarker & AllelicMask::TetraWithGap) {
				// All possible nucleotide characters have been found in the column.
				// No need to check more sequences -> stop the inner loop.
				break;
			}
		}

		// Detect the polymorphic state of the column.
		// This is not necessary for tetrallelic sites since it would have already been marked after
		// the for loop above.
		if (!(allelicMarker & AllelicMask::Tetra)) {
			int nAlleles = 0;
			if (allelicMarker & AllelicMask::Adenine) {
				nAlleles++;
			}
			if (allelicMarker & AllelicMask::Cytosine) {
				nAlleles++;
			}
			if (allelicMarker & AllelicMask::Guanine) {
				nAlleles++;
			}
			if (allelicMarker & AllelicMask::Thymine) {
				nAlleles++;
			}

			switch (nAlleles) {
			case 1:
				allelicMarker |= AllelicMask::Mono;
				break;
			case 2:
				allelicMarker |= AllelicMask::Bi;
				break;
			case 3:
				allelicMarker |= AllelicMask::Tri;
				break;
			default:
				// Do nothing
				break;
			}
		}

		// Save the allelic marker
		this->m_allelicMarkers.push_back(allelicMarker);
	}
}

void Alignment::excludeMonomorphicColumns() {
	// Make sure the m_alignment has been locked before processing it
	this->lock();

	// Delete non-polymorphic columns
	size_t newActiveLen = 0;
	for (const auto& activeColIdx : (*m_activeNucPositions)) {
		auto allelicMarker = this->m_allelicMarkers[activeColIdx];
		if ((allelicMarker & AllelicMask::Bi) || (allelicMarker & AllelicMask::Tri)
			|| (allelicMarker & AllelicMask::Tetra)) {
			(*m_activeNucPositions)[newActiveLen] = activeColIdx;
			newActiveLen++;
		}
	}
	m_activeNucPositions->resize(newActiveLen);

	if (newActiveLen <= 0) {
		throw std::logic_error(
			"The alignment does not contain any polymorphic columns; analysis halted.");
	}
}

const std::vector<SequencePtr>& Alignment::getActiveParents() const {
	return m_parentPool.getActiveSequences();
}

const std::vector<SequencePtr>& Alignment::getActiveChildren() const {
	return m_childPool.getActiveSequences();
}

SequencePool& Alignment::getParentPool() {
	return this->m_parentPool;
}

SequencePool& Alignment::getChildPool() {
	return this->m_childPool;
}

std::set<SequencePtr> Alignment::getAllUsedSequences() const {
	std::set<SequencePtr> allUsedSequences;
	for (auto sequence : m_parentPool.getUsedSequences()) {
		allUsedSequences.insert(sequence);
	}

	if (!m_isSinglePool) {
		for (auto sequence : m_childPool.getUsedSequences()) {
			allUsedSequences.insert(sequence);
		}
	}

	return allUsedSequences;
}
