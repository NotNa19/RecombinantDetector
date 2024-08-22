#pragma once
#include <map>
#include <vector>
#include <stack>

#include "Triplet.h"
#include "Sequence.h"

#include "../app/TextFile.h"

class TripletPool {
public:
	enum class StorageMode {
		BestTriplet,
		AllTriplets
	};

	TripletPool();

	TripletPtr newTriplet(const SequencePtr& child,
		const SequencePtr& dad,
		const SequencePtr& mum);

	void saveTriplet(const SequencePtr& child, const TripletPtr& newTriplet);

	void freeTriplet(TripletPtr& triplet);

	void setStorageMode(StorageMode newStorageMode);

	void writeToFile(TextFile* fileRecombinants, const std::string& separator = ",") const;

	void seekBreakPointPairs(const SequencePtr& child);

	size_t getLongestMinRecLength() const;

	std::map<SequencePtr, std::vector<TripletPtr>> savedTriplets;

private:
	std::stack<TripletPtr> m_freeTriplets;

	StorageMode m_storageMode;

	size_t m_longestMinRecLength;
};
