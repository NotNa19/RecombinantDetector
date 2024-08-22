#pragma once
#include <memory>
#include "Alignment.h"

class AlignmentDescriptor {
public:
    class AllelicStats;
    class PairwiseDistanceStats;
    class TripletCounts;

    AlignmentDescriptor() = delete;

    explicit AlignmentDescriptor(Alignment& alignment);

    const std::unique_ptr<AllelicStats>& getAllelicStats();
    const std::unique_ptr<PairwiseDistanceStats>& getPwDistanceStats();
    const std::unique_ptr<TripletCounts>& getTripletCounts();

private:
    Alignment* const m_alignment;
    std::unique_ptr<AllelicStats> m_allelicStats;
    std::unique_ptr<PairwiseDistanceStats> m_pairwiseDistanceStats;
    std::unique_ptr<TripletCounts> m_tripletCounts;
};

struct AlignmentDescriptor::AllelicStats {
public:

    size_t nMonoallelic;
    size_t nBiallelic;
    size_t nTriallelic;
    size_t nTetrallelic;
    size_t nOnlyGaps;
    size_t nNoGap;

    explicit AllelicStats(const Alignment& alignment);
};

struct AlignmentDescriptor::PairwiseDistanceStats {
public:
    size_t nPwComparisons;
    size_t minPwDist;
    size_t maxPwDist;
    long double meanPwDist;
    long double meanPwDistPerSite;

    explicit PairwiseDistanceStats(const Alignment& alignment);
};

struct AlignmentDescriptor::TripletCounts {
public:
    size_t all;
    size_t used;
    size_t active;

    explicit TripletCounts(const Alignment& alignment);
};
