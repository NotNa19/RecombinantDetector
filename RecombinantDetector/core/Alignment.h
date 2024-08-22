#pragma once
#include <vector>
#include <set>

#include "Sequence.h"
#include "AllelicMask.h"
#include "SequencePool.h"

class AlignmentDescriptor;

class Alignment {
public:
	friend class AlignmentDescriptor;

	Alignment(const Alignment& rhs) = delete;
	Alignment& operator=(const Alignment&) = delete;
	Alignment(Alignment&&) = delete;
	Alignment& operator=(Alignment&&) = delete;
	Alignment();

	/**
	 * Add a sequence to the m_alignment. The sequence will be added to both the child and the parent
	 * pools.
	 * sequencePtr A pointer to the sequence.
	 * asParent A flag that indicates if the sequence should be added to the parent pool.
	 * asChild  A flag that indicates if the sequence should be added to the child pool.
	 */
	void addSequence(const SequencePtr& sequencePtr, const bool& asParent, const bool& asChild);

	void addSequences(const std::vector<SequencePtr>& sequenceList,
		const bool& asParent, const bool& asChild);

	size_t activeLength() const;

	/**
	 * Get the original number of columns of the m_alignment as read from the sequence file
	 * (before shrinking, cutting).
	 * Returns the original number of columns of the m_alignment
	 */
	size_t fullLength() const;

	bool isChildParentFromSamePool() const;

	/**
	 * Lock the m_alignment, i.e. forbidding adding more sequences into the m_alignment.
	 * The locking process also does: [1] populating the active nucleotide-position vector
	 * and [2] scan and mark the allelic status of each m_alignment column.
	 */
	void lock();

	void excludeMonomorphicColumns();

	const std::vector<SequencePtr>& getActiveParents() const;
	const std::vector<SequencePtr>& getActiveChildren() const;

	SequencePool& getParentPool();
	SequencePool& getChildPool();

	std::set<SequencePtr> getAllUsedSequences() const;

private:
	void populatePositionVector();

	void markAllelicStatuses();

private:
	/**
	 * A pointer to the vector that contains active nucleotide-positions.
	 * The vector is managed by the Alignment class.
	 */
	std::shared_ptr<std::vector<size_t>> m_activeNucPositions;

	/**
	 * The vector contains pointers to all sequences in this m_alignment,
	 * including both parent and child sequences.
	 */
	SequencePool m_parentPool;
	SequencePool m_childPool;

	size_t m_fullSequenceLength;

	/** The vector contains allelic status of each column in the m_alignment */
	std::vector<AllelicMask> m_allelicMarkers;

	/**
	 * A flag that indicates if the m_alignment has been locked or it is still open for adding more
	 * sequences. The m_alignment will be locked (i.e. no more sequences can be added into it) as soon
	 * as the processing of the m_alignment (e.g. trimming, cutting) starts.
	 */
	bool m_locked;

	/**
	 * Whether parent and child sequences come from the same sequence pool.
	 */
	bool m_isSinglePool;
};

