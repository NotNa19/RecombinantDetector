#include "AlignmentDescriptor.h"

#include <stdexcept>
#include <limits>

AlignmentDescriptor::AlignmentDescriptor(Alignment& alignment)
    : m_alignment(&alignment),
    m_allelicStats(nullptr), m_pairwiseDistanceStats(nullptr), m_tripletCounts(nullptr) {
}

AlignmentDescriptor::AllelicStats::AllelicStats(const Alignment& alignment)
    : nMonoallelic(0), nBiallelic(0), nTriallelic(0), nTetrallelic(0),
    nOnlyGaps(0), nNoGap(0) {
    // Make sure the m_alignment is locked before calculating the stats
    if (!alignment.m_locked) {
        throw std::logic_error(
            "Allelic statistics cannot be calculated for an unlocked alignment.");
    }

    // Scan through the allelic markers of the ACTIVE columns in the m_alignment
    // and update the statistics
    for (const auto& columnIdx : *(alignment.m_activeNucPositions)) {
        auto allelicMarker = alignment.m_allelicMarkers[columnIdx];
        if (allelicMarker == AllelicMask::Gap) {
            this->nOnlyGaps++;

        }
        else {
            if (!(allelicMarker & AllelicMask::Gap)) {
                this->nNoGap++;
            }

            if (allelicMarker & AllelicMask::Mono) {
                this->nMonoallelic++;
            }
            else if (allelicMarker & AllelicMask::Bi) {
                this->nBiallelic++;
            }
            else if (allelicMarker & AllelicMask::Tri) {
                this->nTriallelic++;
            }
            else if (allelicMarker & AllelicMask::Tetra) {
                this->nTetrallelic++;
            }
        }
    }
}

AlignmentDescriptor::PairwiseDistanceStats::PairwiseDistanceStats(const Alignment& alignment)
    : nPwComparisons(0), minPwDist(std::numeric_limits<size_t>::max()), maxPwDist(0),
    meanPwDist(0.0L), meanPwDistPerSite(0.0L) {
    // Make sure the m_alignment is locked before calculating the stats
    if (!alignment.m_locked) {
        throw std::logic_error(
            "Pairwise-distance statistics cannot be calculated for an unlocked alignment.");
    }

    long double totalPwDist = 0.0L;
    for (const auto& parentSeq : alignment.getActiveParents()) {
        for (const auto& childSeq : alignment.getActiveChildren()) {
            if (parentSeq != childSeq) {
                nPwComparisons++;
                auto newPwDist = childSeq->computeDistance(*parentSeq, true);
                if (newPwDist > 0) {
                    totalPwDist += static_cast<long double>(newPwDist);
                }
                if (minPwDist > newPwDist) {
                    minPwDist = newPwDist;
                }
                if (maxPwDist < newPwDist) {
                    maxPwDist = newPwDist;
                }
            }
        }
    }
    meanPwDist = totalPwDist / static_cast<long double>(nPwComparisons);
    meanPwDistPerSite = meanPwDist / static_cast<long double>(alignment.activeLength());
}

const std::unique_ptr<AlignmentDescriptor::AllelicStats>&
AlignmentDescriptor::getAllelicStats() {
    if (m_allelicStats == nullptr) {
        m_allelicStats = std::make_unique<AllelicStats>(*m_alignment);
    }
    return m_allelicStats;
}

const std::unique_ptr<AlignmentDescriptor::PairwiseDistanceStats>&
AlignmentDescriptor::getPwDistanceStats() {
    if (m_pairwiseDistanceStats == nullptr) {
        m_pairwiseDistanceStats = std::make_unique<PairwiseDistanceStats>(*m_alignment);
    }
    return m_pairwiseDistanceStats;
}

AlignmentDescriptor::TripletCounts::TripletCounts(const Alignment& alignment)
    : all(0), used(0), active(0) {

    // Make sure the m_alignment is locked before calculating the stats
    if (!alignment.m_locked) {
        throw std::logic_error("Triplet count cannot be performed on an unlocked alignment.");
    }

    auto nAllChildren = alignment.m_childPool.allSequencesSize();
    auto nUsedChildren = alignment.m_childPool.usedSequencesSize();
    auto nActiveChildren = alignment.m_childPool.activeSequencesSize();

    auto nAllParents = alignment.m_parentPool.allSequencesSize();
    auto nUsedParents = alignment.m_parentPool.usedSequencesSize();
    auto nActiveParents = alignment.m_parentPool.activeSequencesSize();
    if (alignment.isChildParentFromSamePool()) {
        nAllParents--;
        nUsedParents--;
        nActiveParents--;
    }

    // Calculate the number of triplets that can be formed from the m_alignment
    if (nAllParents >= 2 && nAllChildren >= 1) {
        this->all = nAllChildren * nAllParents * (nAllParents - 1);
    }
    if (nUsedParents >= 2 && nUsedChildren >= 1) {
        this->used = nUsedChildren * nUsedParents * (nUsedParents - 1);
    }
    if (nActiveParents >= 2 && nActiveChildren >= 1) {
        this->active = nActiveChildren * nActiveParents * (nActiveParents - 1);
    }
}

const std::unique_ptr<AlignmentDescriptor::TripletCounts>& AlignmentDescriptor::getTripletCounts() {
    if (m_tripletCounts == nullptr) {
        m_tripletCounts = std::make_unique<TripletCounts>(*m_alignment);
    }
    return m_tripletCounts;
}
