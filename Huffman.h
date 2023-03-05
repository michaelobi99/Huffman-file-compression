#pragma once
#include "BitIO.h"
#include <queue>
#include <bitset>
#include <ranges>
#include <iostream>

struct treeNode {
	unsigned char symbol;
	std::uint32_t symbolFrequency;
	treeNode* child_0{ nullptr };
	treeNode* child_1{ nullptr };
	treeNode(unsigned char val, std::uint32_t freq) : symbol{ val }, symbolFrequency{ freq }{}
};

struct treeNodeComparator {
	bool operator() (treeNode* lhs, treeNode* rhs) {
		return lhs->symbolFrequency > rhs->symbolFrequency;
	}
};

using treeQueue = std::priority_queue<treeNode*, std::vector<treeNode*>, treeNodeComparator>;

using uChar = unsigned char;

#define ENDOFSTREAM treeNode(uChar(254), 1)
#define ENDOFCOUNT 0

struct code {
	std::uint32_t symbolCode;
	std::uint32_t bitLength;
};


//function to count the relative frequencies of the symbols.
void countBytes(std::fstream& file, std::unique_ptr<std::uint32_t[]>& counts) {
	char ch{};
	while (file.get(ch)) {
		counts[(int)ch]++;
		if (counts[int(ch)] == 256) //won't fit in a char, hence i scale down all the counts, to enable easy reading and writing
		{
			for (auto elem : std::ranges::iota_view(0, 256)) {
				if (counts[elem] > 0)
					counts[elem] = (counts[elem] + 1) / 2;
			}
		}
	}
	file.clear();
	file.seekg(0, std::ios_base::beg);
}

void createNodes(std::unique_ptr<std::uint32_t[]>& counts, treeQueue& nodes) {
	for (auto i : std::ranges::iota_view(0, 256)) {
		if (counts[i] > 0) {
			nodes.push(new treeNode((uChar)i, counts[i]));
		}
	}
	//254 is used as the EOF symbol
	nodes.push(new ENDOFSTREAM);
}

void outputCounts(std::unique_ptr <stl::BitFile>& output, std::unique_ptr<std::uint32_t[]>& counts) {
	std::cout << "Output count\n";
	for (auto i : std::ranges::iota_view(0, 256)) {
		if (counts[i] > 0) {
			output->file.put(counts[i]);
			output->file.put(i);
			std::cout<< i<<" " << counts[i] << "\n";
		}
	}
	output->file.put(ENDOFCOUNT);//end of counts symbol
}

void inputCounts(std::unique_ptr <stl::BitFile>& input, treeQueue& nodes) {
	std::cout << "input count\n";
	std::uint32_t count{}, symbol{};
	while (true) {
		count = input->file.get();
		if (count == ENDOFCOUNT)
			break;
		symbol = input->file.get();
		nodes.push(new treeNode(uChar(symbol), count));
		std::cout << symbol << " " << count<< "\n";
	}
	nodes.push(new ENDOFSTREAM);
}

treeNode* build_tree(treeQueue& nodes) {
	treeNode* child_0{ nullptr }, * child_1{ nullptr };
	while (nodes.size() > 1) {
		child_0 = nodes.top();
		nodes.pop();
		child_1 = nodes.top();
		nodes.pop();
		auto newNode = new treeNode(uChar{}, child_0->symbolFrequency + child_1->symbolFrequency);
		newNode->child_0 = child_0;
		newNode->child_1 = child_1;
		nodes.push(newNode);
	}
	return nodes.top();
}

void convertTreeToCode(std::unique_ptr<code[]>& codes, std::uint32_t codeSoFar, std::uint32_t bitCount, treeNode* node) {
	if (!node->child_0) {//if node is a leaf
		codes[(int)(node->symbol)].symbolCode = codeSoFar;
		codes[(int)(node->symbol)].bitLength = bitCount;
		return;
	}
	codeSoFar <<= 1;
	bitCount++;
	convertTreeToCode(codes, codeSoFar, bitCount, node->child_0);
	convertTreeToCode(codes, codeSoFar | 1, bitCount, node->child_1);
}

void compressData(std::fstream& input, std::unique_ptr <stl::BitFile>& output, std::unique_ptr<code[]>& codes) {
	char c;
	int counter = 0;
	while (input.get(c)) {
		stl::outputBits(output, codes[(int)c].symbolCode, codes[(int)c].bitLength);
		counter += codes[(int)c].bitLength;
	}
	stl::outputBits(output, codes[254].symbolCode, codes[254].bitLength);
	if (output->mask != 0){
		output->file<<(char)output->rack;
	}
}

void expandData(std::unique_ptr <stl::BitFile>& input, std::fstream&  output, treeNode* rootNode) {
	while (true) {
		treeNode* node = rootNode;
		while (node->child_0) {//while not at a leaf node
			if (stl::inputBit(input)) {
				node = node->child_1;
			}
			else {
				node = node->child_0;
			}
		}
		if (node->symbol == ENDOFSTREAM.symbol)
			break;
		output.put(node->symbol);
	}
}

void freeMemory(treeNode* root) {
	if (root) {
		freeMemory(root->child_0);
		freeMemory(root->child_1);
		delete root;
	}
}

void compressFile(std::fstream& input, std::unique_ptr <stl::BitFile>& output) {
	std::unique_ptr<std::uint32_t[]> counts = std::make_unique<std::uint32_t[]>(256);
	for (int i{ 0 }; i < 256; ++i) 
		counts[i] = 0;
	treeQueue nodes;
	std::unique_ptr<code[]> codes = std::make_unique<code[]>(257);
	countBytes(input, counts);
	createNodes(counts, nodes);//creates nodes of symbols with > 0 counts
	outputCounts(output, counts);
	treeNode* rootNode = build_tree(nodes);
	convertTreeToCode(codes, 0, 0, rootNode);
	compressData(input, output, codes);
	freeMemory(rootNode);
}

void expandFile(std::unique_ptr <stl::BitFile>& input, std::fstream& output) {
	treeQueue nodes;
	treeNode* rootNode;
	inputCounts(input, nodes);
	rootNode = build_tree(nodes);
	expandData(input, output, rootNode);
	freeMemory(rootNode);
}