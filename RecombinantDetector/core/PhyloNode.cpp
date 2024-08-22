#include "PhyloNode.h"

PhyloNode::PhyloNode(SequencePtr seq):m_seq(seq) {
	mutations.resize(m_seq->activeLength());
}

void PhyloNode::addChild(spPhyloNode child) {
	m_children.emplace_back(child);
}