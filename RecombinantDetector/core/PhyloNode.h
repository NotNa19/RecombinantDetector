#pragma once
#include "Sequence.h"

#include <vector>
#include <memory>
#include <bitset>
#include <unordered_set>

class PhyloNode;
using spPhyloNode = std::shared_ptr<PhyloNode>;
using CladeCode = size_t;

class PhyloNode
{
public:
	PhyloNode(SequencePtr seq);
	~PhyloNode() = default;
	PhyloNode(PhyloNode const& other) = delete;
	PhyloNode& operator=(PhyloNode const& other) = delete;

	PhyloNode(PhyloNode&& other) = delete;
	PhyloNode& operator=(PhyloNode&& other) = delete;

	std::string name() const {
		return m_seq->name();
	}

	SequencePtr sequence() {
		return m_seq;
	}

	const std::vector<spPhyloNode>& children() const {
		return m_children;
	}

	void addChild(spPhyloNode child);

	std::vector<bool> mutations;
	const static size_t nucsNum;
	size_t activeMut = 0;

private:
	SequencePtr m_seq;
	std::vector<spPhyloNode> m_children;
};

