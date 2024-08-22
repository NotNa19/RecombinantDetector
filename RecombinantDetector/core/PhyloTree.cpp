#pragma once
#include <fstream> 
#include <iostream> 
#include <cstring>
#include <sstream>
#include <queue>

#include "PhyloTree.h"
#include "PhyloNode.h"
#include "utils/Utils.h"

// TODO: refactor this class using new structures
PhyloTree::PhyloTree(SequencePtr rootSeq) {
	m_root = std::make_shared<PhyloNode>(rootSeq);
}

void PhyloTree::build(const std::vector<SequencePtr>& seqs) {
	// TODO: throw an error
	if (!m_root) std::cout << "ERROR: NULL ROOT";

	std::vector<std::vector<size_t>> childIndexes;
	m_allNodes.reserve(seqs.size() + 1);
	m_allNodes.push_back(m_root);

	for (size_t i = 0; i < seqs.size(); i++) {
		processNextSeq(seqs[i]);

		if (m_allNodes.size() % 100 == 0) {
			std::cout << "Tree Building. Computed " << m_allNodes.size() << "/" << seqs.size() << "\n";
		}
	}

	/*std::ifstream inputFile(path);
	size_t nodesCount, nonTerminalNodeCount;
	inputFile >> nodesCount >> nonTerminalNodeCount;
	inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	std::vector<std::vector<size_t>> childIndexes;
	for (size_t i = 0; i < nonTerminalNodeCount; i++) {

		std::string line;
		std::getline(inputFile, line);
		std::istringstream iss(line);
		size_t val;
		std::vector<size_t> temp;
		while (iss >> val) {
			temp.push_back(val);
		}
		childIndexes.push_back(std::move(temp));
	}

	inputFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::vector<spPhyloNode> nodes;
	nodes.reserve(nodesCount);
	for (size_t i = 0; i < nodesCount; i++) {
		std::string line;
		std::getline(inputFile, line);
		PhyloSequence seqRoot(line.size());
		for (size_t j = 0; j < seqRoot.size; j++) {
			seqRoot.sequence[j] = line[j] - '0';
		}
		nodes.emplace_back(std::make_shared<PhyloNode>(std::move(seqRoot)));
		nodes.back()->setCladeCode(i);
	}

	for (size_t i = 0; i < nonTerminalNodeCount; i++) {
		const auto& currentChildren = childIndexes[i];
		const auto& parent = nodes[i];
		for (auto idx : currentChildren)
			parent->addChild(nodes[idx]);
	}

	m_root = nodes[0];*/

	std::cout << "TOTAL: " << m_allNodes.size() << "\n";
}

void PhyloTree::computeMutationIndexes() {
	std::queue<spPhyloNode> queue;
	for (auto& child : m_root->children())
		queue.push(child);

	while (!queue.empty()) {
		spPhyloNode node = queue.front();
		queue.pop();
		computeMutationIndexes(node);
		for (auto& child : node->children())
			queue.push(child);
	}
}

void PhyloTree::computeMutationIndexes(spPhyloNode node) {
	const auto& nucs = node->sequence()->nucleotides();
	const auto& rootNucs = m_root->sequence()->nucleotides();
	if (nucs.size() != rootNucs.size()) {
		std::cout << "ERROR: DIFFERENT NUCLEOTIDES LENGHT. " << nucs.size() << " vs " << rootNucs.size() << "\n";
		return;
	}

	auto& nodeMutations = node->mutations;

	for (size_t i = 0; i < nucs.size(); i++) {
		if (nucs[i] != rootNucs[i]) {
			nodeMutations[i] = true;
			node->activeMut++;
		}
	}
}

void PhyloTree::processNextSeq(SequencePtr seq) {
	spPhyloNode newNode = std::make_shared<PhyloNode>(seq);
	computeMutationIndexes(newNode);
	spPhyloNode nearestNode = nullptr;
	size_t minDist = std::numeric_limits<size_t>::max();

	auto& newNodeMutations = newNode->mutations;

	for (const auto& currentNode : m_allNodes) {
		size_t dist = 0;

		int muts = newNode->activeMut - currentNode->activeMut;
		if (std::abs(muts) > 5000) {
			continue;
		}

		dist += newNode->activeMut;
		dist += currentNode->activeMut;
		size_t agreeCount = 0;
		size_t disagreeCount = 0;

		auto& currentNodeMutations = currentNode->mutations;

		const auto& newNucs = newNode->sequence()->nucleotides();
		const auto& currNucs = currentNode->sequence()->nucleotides();

		for (size_t i = 0; i < newNode->mutations.size(); i++) {
			if (currentNodeMutations[i] && newNodeMutations[i])
			{
				if (newNucs[i] == currNucs[i])
					agreeCount += 1;
				else
					disagreeCount += 1;
			}
		}

		dist -= 2 * agreeCount + disagreeCount;

		if (dist < minDist) {
			minDist = dist;
			nearestNode = currentNode;
		}
	}

	if (!nearestNode) {
		std::cout << "ERROR DURING NEAREST NODE COMPUTATION. NODE WILL BE SKIPPED " << newNode->activeMut << "\n";
		return;
	}

	nearestNode->addChild(newNode);
	m_allNodes.push_back(newNode);

}

PhyloTree::QueryOutput PhyloTree::findNearestNode(spPhyloNode node) {


	PhyloTree::QueryOutput algOutput;
	/*
	computeMutationIndexes(node);
	std::queue<spPhyloNode> queue;
	auto& nodeMuts = node->sequence().mutations;
	queue.push(m_root);
	size_t minDist = std::numeric_limits<size_t>::max();
	CladeCode nearestClade;
	while (!queue.empty()) {
		auto current = queue.front();
		queue.pop();
		auto currentMuts = current->sequence().mutations;

		size_t dist = 0;
		dist += node->sequence().activeMut;
		dist += current->sequence().activeMut;
		size_t agreeCount = 0;
		size_t disagreeCount = 0;

		for (size_t i = 0; i < node->sequence().size; i++) {
			if (currentMuts[i] && nodeMuts[i])
			{
				if (node->sequence().sequence[i] == current->sequence().sequence[i])
					agreeCount += 1;
				else
					disagreeCount += 1;
			}
		}


		dist -= 2 * agreeCount + disagreeCount;

		if (minDist > dist) {
			minDist = dist;
			nearestClade = current->cladeCode();
		}
		algOutput.distanceMap.try_emplace(current->cladeCode(), dist);

		for (auto& child : current->children())
			queue.push(child);
	}
	algOutput.minDistance = minDist;
	algOutput.nearestNodeCode = nearestClade;*/

	return algOutput;
}