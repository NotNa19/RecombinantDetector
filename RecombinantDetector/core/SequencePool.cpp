#include "SequencePool.h"

#include <stdexcept>

#include "../utils/numeric_types.h"

SequencePool::SequencePool()
    : m_allSequences(), m_usedSequences(), m_activeSequences(), m_sequenceStatuses() {
}

void SequencePool::addSequence(const SequencePtr& sequencePtr) {
    if (!this->m_allSequences.empty()
        && sequencePtr->activeLength() != this->getSequenceActiveLength()) {
        throw std::length_error(
            "Unmatched sequence length is found in sequence '" + sequencePtr->name()
            + "' (expected: " + std::to_string(this->getSequenceActiveLength())
            + ", received: " + std::to_string(sequencePtr->activeLength()) + ").");

    }
    if (findSequenceByName(sequencePtr->name()) != nullptr) {
        throw std::runtime_error("Duplicate sequence name detected: "
            + sequencePtr->name() + ".");

    }
    else {
        this->m_allSequences.push_back(sequencePtr);
        this->m_usedSequences.push_back(sequencePtr);
        this->m_activeSequences.push_back(sequencePtr);
        this->m_sequenceStatuses.push_back(SequenceStatus::Active);
    }
}

SequencePtr SequencePool::findSequenceByName(const std::string& sequenceName) const {
    for (auto sequence : m_allSequences) {
        if (sequence->name() == sequenceName) {
            return sequence;
        }
    }
    return nullptr;
}

size_t SequencePool::findSequenceIdxByName(const std::string& sequenceName) const {
    auto seqCount = m_allSequences.size();
    for (size_t seqIdx = 0; seqIdx < seqCount; seqIdx++) {
        if (m_allSequences[seqIdx]->name() == sequenceName) {
            return seqIdx;
        }
    }
    return ULong::NOT_SET;
}

size_t SequencePool::getSequenceActiveLength() const {
    if (m_activeSequences.empty()) {
        return 0;
    }
    else {
        return m_activeSequences.front()->activeLength();
    }
}

size_t SequencePool::getSequenceFullLength() const {
    if (m_allSequences.empty()) {
        return 0;
    }
    else {
        return m_allSequences.front()->fullLength();
    }
}

void SequencePool::cacheSequenceStatuses() {
    m_usedSequences.clear();
    m_activeSequences.clear();

    auto seqCount = m_allSequences.size();
    for (size_t seqIdx = 0; seqIdx < seqCount; seqIdx++) {
        if (m_sequenceStatuses[seqIdx] == SequenceStatus::Active) {
            m_activeSequences.push_back(m_allSequences[seqIdx]);
            m_usedSequences.push_back(m_allSequences[seqIdx]);
        }
        else if (m_sequenceStatuses[seqIdx] == SequenceStatus::Inactive) {
            m_usedSequences.push_back(m_allSequences[seqIdx]);
        }
    }
}

void SequencePool::activateSequencesBy1BasedIdxRange(const size_t& startIdx,
    const size_t& endIdx) {
    if (!isSet(startIdx) || !isSet(endIdx)) {
        return;
    }

    auto seqCount = m_allSequences.size();
    if (1 > startIdx || startIdx > endIdx || endIdx > seqCount) {
        throw std::range_error(
            "Invalid 1-based index range: [" + std::to_string(startIdx)
            + ", " + std::to_string(endIdx) + "].");
    }

    return activateSequencesByIdxRange(startIdx - 1, endIdx - 1);
}

void SequencePool::activateSequencesByIdxRange(const size_t& startIdx,
    const size_t& endIdx) {
    if (!isSet(startIdx) || !isSet(endIdx)) {
        return;
    }

    auto seqCount = m_allSequences.size();
    if (0 > startIdx || startIdx > endIdx || endIdx >= seqCount) {
        throw std::range_error(
            "Invalid 0-based index range: [" + std::to_string(startIdx)
            + ", " + std::to_string(endIdx) + "].");
    }

    for (size_t idx = 0; idx < seqCount; idx++) {
        if (m_sequenceStatuses[idx] == SequenceStatus::Excluded) {
            continue;
        }

        if (startIdx <= idx && idx <= endIdx) {
            m_sequenceStatuses[idx] = SequenceStatus::Active;
        }
        else {
            m_sequenceStatuses[idx] = SequenceStatus::Inactive;
        }
    }

    cacheSequenceStatuses();
}

void SequencePool::activateSequencesByNames(const std::vector<std::string>& sequenceNames) {
    auto seqCount = m_allSequences.size();

    // Deactivate all used sequences
    for (size_t seqIdx = 0; seqIdx < seqCount; seqIdx++) {
        if (m_sequenceStatuses[seqIdx] != SequenceStatus::Excluded) {
            m_sequenceStatuses[seqIdx] = SequenceStatus::Inactive;
        }
    }

    // Activate the sequences whose names appear in the given list
    for (const auto& name : sequenceNames) {
        auto seqIdx = findSequenceIdxByName(name);
        if (seqIdx < seqCount) {
            if (m_sequenceStatuses[seqIdx] != SequenceStatus::Excluded) {
                m_sequenceStatuses[seqIdx] = SequenceStatus::Active;
            }
        }
        else {
            throw std::runtime_error("Sequence not found: " + name + ".");
        }
    }

    cacheSequenceStatuses();
}

size_t SequencePool::allSequencesSize() const {
    return m_allSequences.size();
}

size_t SequencePool::usedSequencesSize() const {
    return m_usedSequences.size();
}

size_t SequencePool::activeSequencesSize() const {
    return m_activeSequences.size();
}

const std::vector<SequencePtr>& SequencePool::getActiveSequences() const {
    return this->m_activeSequences;
}

const std::vector<SequencePtr>& SequencePool::getUsedSequences() const {
    return this->m_usedSequences;
}

size_t SequencePool::distinctSequencesSize() const {
    size_t distinctSeqCount = 0;
    auto allSeqCount = m_allSequences.size();

    for (size_t seqIdx = 0; seqIdx < allSeqCount; seqIdx++) {
        if (m_sequenceStatuses[seqIdx] == SequenceStatus::Excluded) {
            continue;
        }

        auto sequence = m_allSequences[seqIdx];
        bool isDistinct = true;
        for (size_t anotherSeqIdx = 0; anotherSeqIdx < allSeqCount; anotherSeqIdx++) {
            if (anotherSeqIdx == seqIdx
                || m_sequenceStatuses[anotherSeqIdx] == SequenceStatus::Excluded) {
                continue;
            }
            if (sequence->isSimilarWithGaps(*(m_allSequences[anotherSeqIdx]))) {
                isDistinct = false;
                break;
            }
        }
        if (isDistinct) {
            distinctSeqCount++;
        }
    }

    return distinctSeqCount;
}

void SequencePool::excludeNonDistinctSequences() {
    auto allSeqCount = m_allSequences.size();

    for (size_t seqIdx = 0; seqIdx < allSeqCount; seqIdx++) {
        if (m_sequenceStatuses[seqIdx] == SequenceStatus::Excluded) {
            continue;
        }

        auto sequence = m_allSequences[seqIdx];
        for (size_t anotherSeqIdx = 0; anotherSeqIdx < allSeqCount; anotherSeqIdx++) {
            if (anotherSeqIdx == seqIdx
                || m_sequenceStatuses[anotherSeqIdx] == SequenceStatus::Excluded) {
                continue;
            }
            if (m_allSequences[anotherSeqIdx]->isSimilarWithGaps(*sequence)) {
                m_sequenceStatuses[anotherSeqIdx] = SequenceStatus::Excluded;
            }
        }
    }

    cacheSequenceStatuses();
}

void SequencePool::excludeNeighboringSequences(const size_t& maxNeighborDistance) {
    if (maxNeighborDistance == 0) {
        return excludeNonDistinctSequences();
    }

    auto seqCount = m_allSequences.size();

    // Count the number of gaps in each sequence
    std::vector<size_t> gapCounts(seqCount, 0);
    for (size_t seqIdx = 0; seqIdx < seqCount; seqIdx++) {
        if (m_sequenceStatuses[seqIdx] == SequenceStatus::Excluded) {
            continue;
        }
        gapCounts[seqIdx] = m_allSequences[seqIdx]->countGaps();
    }

    // Find neighboring sequences and mark those with more gap as unused.
    // I.e. keep only 1 sequence (that has the fewest number of gaps) for each neighboring group.
    for (size_t seqIdx = 0; seqIdx < (seqCount - 1); seqIdx++) {
        if (m_sequenceStatuses[seqIdx] == SequenceStatus::Excluded) {
            continue;
        }

        auto sequence = m_allSequences[seqIdx];
        for (size_t anotherSeqIdx = (seqIdx + 1);
            anotherSeqIdx < seqCount; anotherSeqIdx++) {
            if (m_sequenceStatuses[anotherSeqIdx] == SequenceStatus::Excluded) {
                continue;
            }

            auto anotherSeq = m_allSequences[anotherSeqIdx];
            if (sequence->computeDistance(*anotherSeq) <= maxNeighborDistance) {
                if (gapCounts[seqIdx] <= gapCounts[anotherSeqIdx]) {
                    m_sequenceStatuses[anotherSeqIdx] == SequenceStatus::Excluded;
                }
                else {
                    m_sequenceStatuses[seqIdx] == SequenceStatus::Excluded;
                    break; 
                }
            }
        }
    }

    cacheSequenceStatuses();
}
