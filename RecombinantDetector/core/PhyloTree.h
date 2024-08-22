#pragma once
#include <filesystem>
#include <unordered_map>

#include "PhyloNode.h"
#include "Sequence.h"

namespace fs = std::filesystem;
class PhyloTree
{
public:
	struct QueryOutput {
		CladeCode nearestNodeCode;
		size_t minDistance;
		std::unordered_map<CladeCode, size_t> distanceMap;
	};

	PhyloTree(SequencePtr rootSeq);
	~PhyloTree() = default;
	PhyloTree(PhyloTree const& other) = delete;
	PhyloTree& operator=(PhyloTree const& other) = delete;

	PhyloTree(PhyloTree&& other) = delete;
	PhyloTree& operator=(PhyloTree&& other) = delete;


	void build(const std::vector<SequencePtr>& seqs);

	spPhyloNode root() const {
		return m_root;
	}

	// Computes mutation positions relative to the root for all nodes
	void computeMutationIndexes();
	// Computes mutation positions relative to the root for the input node
	void computeMutationIndexes(spPhyloNode node);

	QueryOutput findNearestNode(spPhyloNode node);

private:
	void processNextSeq(SequencePtr);

private:
	spPhyloNode m_root;
	std::vector<spPhyloNode> m_allNodes;
};

