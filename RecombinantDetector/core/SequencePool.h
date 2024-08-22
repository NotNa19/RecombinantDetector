#pragma once
#include "Sequence.h"

#include <vector>
#include <memory>

class Alignment;

class SequencePool {
public:

	enum class SequenceStatus {
		Excluded,
		Inactive,
		Active
	};

	friend class Alignment;

	SequencePool(const SequencePool& rhs) = delete;
	SequencePool& operator=(const SequencePool&) = delete;
	SequencePool(SequencePool&&) = delete;
	SequencePool& operator=(SequencePool&&) = delete;

	// Allowed constructor
	explicit SequencePool();

	void addSequence(const SequencePtr& sequencePtr);

	SequencePtr findSequenceByName(const std::string& sequenceName) const;
	size_t findSequenceIdxByName(const std::string& sequenceName) const;

	size_t getSequenceActiveLength() const;
	size_t getSequenceFullLength() const;

	const std::vector<SequencePtr>& getActiveSequences() const;
	const std::vector<SequencePtr>& getUsedSequences() const;

	/**
	 * Mark sequences whose 1-based indices are between the startIdx and
	 * the endIdx inclusively as active. Used sequences whose indices are outside this
	 * range will be marked as inactive.
	 * @param startIdx  The 1-based index of the first child sequence to be activated.
	 * @param endIdx    The 1-based index of the last child sequence to be activated.
	 * @note Sequences whose use status is already set as SequenceStatus::UNUSED will not be
	 *       affected by this method.
	 */
	void activateSequencesBy1BasedIdxRange(const size_t& startIdx,
		const size_t& endIdx);

	/**
	 * Mark sequences whose 0-based indices are between the startIdx and
	 * the endIdx inclusively as active. Used sequences whose indices are outside this
	 * range will be marked as inactive.
	 * @param startIdx  The 0-based index of the first child sequence to be activated.
	 * @param endIdx    The 0-based index of the last child sequence to be activated.
	 * @note Sequences whose use status is already set as SequenceStatus::UNUSED will not be
	 *       affected by this method.
	 */
	void activateSequencesByIdxRange(const size_t& startIdx,
		const size_t& endIdx);

	/**
	 * Set the active status of sequences whose names are in the given list. The active status of
	 * the other sequences will be set at the opposite value.
	 * @param sequenceNames The names of the sequences whose active statuses are to be set.
	 * @param useStatus The use status to be set for the sequences in the given range. This status
	 *                  must be either UseStatus::ACTIVE or UseStatus::INACTIVE.
	 * @note Sequences whose use status is already set as UseStatus::UNUSED will not be affected
	 *       by this method.
	 */
	void activateSequencesByNames(const std::vector<std::string>& sequenceNames);

	size_t allSequencesSize() const;
	size_t usedSequencesSize() const;
	size_t activeSequencesSize() const;
	size_t distinctSequencesSize() const;

	/**
	 * Mark non-distinct sequences as unused.
	 * When 2 sequences of 0 nucleotide distance are found, the one with fewer gaps will be
	 * preserved.
	 */
	void excludeNonDistinctSequences();

	/**
	 * Mark neighboring sequences as unused. For each group of neighboring sequences,
	 * only 1 sequence that has the fewest number of gaps will be kept.
	 * @param maxNeighborDistance The maximum number of nucleotide differences that a pair of
	 *                            neighboring sequences can have.
	 */
	void excludeNeighboringSequences(const size_t& maxNeighborDistance);

private:
	void cacheSequenceStatuses();

private:
	std::vector<SequencePtr> m_allSequences;
	std::vector<SequencePtr> m_usedSequences;
	std::vector<SequencePtr> m_activeSequences;
	std::vector<SequenceStatus> m_sequenceStatuses;


};
