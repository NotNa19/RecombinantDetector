#pragma once
#include <string>
#include <filesystem>

#include "../core/PhyloNode.h"
#include "../core/PhyloTree.h"

namespace Utils
{
	void outputTree(const PhyloTree& tree);
	std::string mutationIndexesToString(spPhyloNode node);
	void outputMutationIndexes(const PhyloTree& tree);

	spPhyloNode readNodeFromFile(fs::path path);

	std::string extractCladeNameGenBank(const std::string& seqName);
};

