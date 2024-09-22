#include "TripletPool.h"

#include <iostream>

#include "TripletPool.h"

#include "../app/UserSettings.h"

TripletPool::TripletPool()
	: savedTriplets(), m_freeTriplets(), m_storageMode(StorageMode::BestTriplet),
	m_longestMinRecLength(0) {
}

void TripletPool::setStorageMode(TripletPool::StorageMode newStorageMode) {
	m_storageMode = newStorageMode;
}

TripletPtr TripletPool::newTriplet(const SequencePtr& child,
	const SequencePtr& dad,
	const SequencePtr& mum) {
	TripletPtr triplet;
	if (!m_freeTriplets.empty()) {
		triplet = m_freeTriplets.top();
		m_freeTriplets.pop();
	}
	else {
		triplet = Triplet::create();
	}

	triplet->reassign(child, dad, mum);
	return triplet;
}

void TripletPool::saveTriplet(const SequencePtr& child, const TripletPtr& newTriplet) {
	if (savedTriplets.find(child) != savedTriplets.end()) {
		switch (m_storageMode)
		{
		case TripletPool::StorageMode::BestTriplet:
		{
			auto currentBestTriplet = savedTriplets[child][0];
			auto worseTriplet = newTriplet;
			if (newTriplet->statisticallyBetter(*currentBestTriplet)) {
				savedTriplets[child][0] = newTriplet;
				worseTriplet = currentBestTriplet;
			}
			freeTriplet(worseTriplet);
			break;
		}
		case TripletPool::StorageMode::AllTriplets:
			savedTriplets[child].push_back(newTriplet);
			break;
		default:
			break;
		}
	}
	else {
		savedTriplets[child] = std::vector<TripletPtr>{ newTriplet };
	}
}

void TripletPool::freeTriplet(TripletPtr& triplet) {
	m_freeTriplets.push(triplet);
	triplet.reset();
}

void TripletPool::writeToFile(TextFile* fileRecombinants, const std::string& separator) const {
	if (fileRecombinants == nullptr) {
		return;
	}

	if (UserSettings::instance().simplifiedOutput) {
		fileRecombinants->writeLine(
			"Parent1" + separator + "Parent2" + separator + "Child" + separator + "log10(p)" + separator +
			"Dunn_Sidak_Corr(p)" + separator +
			"Min_Rec_Length" + separator + "Breakpoints"
		);
	}
	else {
		fileRecombinants->writeLine(
			"Parent1" + separator + "Parent2" + separator + "Child" + separator +
			"m" + separator + "n" + separator + "k" + separator +
			"p" + separator + "HS?" + separator + "log10(p)" + separator +
			"Dunn_Sidak_Corr(p)" + separator + "DS(p)" + separator +
			"Min_Rec_Length" + separator + "Breakpoints"
		);
	}


	for (const auto& dataPair : savedTriplets) {
		for (const auto& triplet : dataPair.second) {
			fileRecombinants->writeLine(triplet->toString(separator));
		}
	}

	fileRecombinants->close();
}

void TripletPool::seekBreakPointPairs(const SequencePtr& child) {
	if (savedTriplets.find(child) == savedTriplets.end()) {
		return;
	}

	for (const auto& triplet : savedTriplets[child]) {
		triplet->seekBreakPointPairs();
		if (m_longestMinRecLength < triplet->m_minRecombinantLength) {
			m_longestMinRecLength = triplet->m_minRecombinantLength;
		}
	}
}

size_t TripletPool::getLongestMinRecLength() const {
	return m_longestMinRecLength;
}

