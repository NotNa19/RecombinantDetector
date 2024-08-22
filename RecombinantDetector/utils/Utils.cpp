#include "Utils.h"

#include <sstream>
#include <fstream> 
#include <iostream>
#include <queue>
#include <utility>

#include <cstring>
#include <cerrno>

#include <vector>

#include "../app/TextFile.h"

// TODO: Need some refactoring using new structures
/*
void Utils::outputTree(const PhyloTree& tree) {
	const auto& root = tree.root();

	std::cout << "Root: " << "Clade = " << root.cladeCode() << " Seq: " << seqToStr(root.sequence())<<"\n";

	std::queue<std::pair<spPhyloNode, CladeCode>> queue;
	for (const auto& child : root.children())
		queue.push({ child,  root.cladeCode() });

	while (!queue.empty()) {
		CladeCode parentCode = queue.front().second;
		spPhyloNode node = queue.front().first;
		queue.pop();
		std::cout << "Child of: " << parentCode<< " Clade = " << node->cladeCode() << " Seq: " << seqToStr(node->sequence())<<"\n";
		for (const auto& child : node->children())
			queue.push({ child,  node->cladeCode() });
	}
}

std::string Utils::mutationIndexesToString(spPhyloNode node) {
	std::stringstream ss;
	for (size_t i = 0; i < node->sequence().size; i++) {
		if (node->sequence().mutations[i])
			ss << i << " ";
	}

	return ss.str();
}

void Utils::outputMutationIndexes(const PhyloTree& tree) {
	const auto& root = tree.root();

	std::queue<std::pair<spPhyloNode, CladeCode>> queue;
	for (const auto& child : root.children())
		queue.push({ child,  root.cladeCode() });

	while (!queue.empty()) {
		CladeCode parentCode = queue.front().second;
		spPhyloNode node = queue.front().first;
		queue.pop();
		std::cout << "Child of: " << parentCode << " Clade = " << node->cladeCode() << " Mut indexes: " << mutationIndexesToString(node) << "\n";
		for (const auto& child : node->children())
			queue.push({ child,  node->cladeCode() });
	}
}

spPhyloNode Utils::readNodeFromFile(fs::path path) {

	std::ifstream inputFile(path);
	std::string line;
	std::getline(inputFile, line);
    PhyloSequence seqRoot(line.size());
	for (size_t j = 0; j < seqRoot.size; j++) {
		seqRoot.sequence[j] = line[j] - '0';
	}
	
	spPhyloNode result = std::make_shared<PhyloNode>(std::move(seqRoot));
	return result;
}*/

namespace Utils {

    std::string extractCladeNameGenBank(const std::string& seqName) {
        size_t firstDelimPos = seqName.find('|');
        if (firstDelimPos == std::string::npos) {
            std::cout << "Cant extract clade";
            return "DUMMY CLADE";
        }

        size_t secondDelimPos = seqName.find('|', firstDelimPos + 1);
        if (secondDelimPos == std::string::npos) {
            std::cout << "Cant extract clade";
            return "DUMMY CLADE";
        }

        size_t thirdDelimPos = seqName.find('|', secondDelimPos + 1);
        if (thirdDelimPos == std::string::npos) {
            std::cout << "Cant extract clade";
            return "DUMMY CLADE";
        }

        std::string cladeName = seqName.substr(secondDelimPos + 1, thirdDelimPos - secondDelimPos - 1);

        return cladeName;
    }
}
