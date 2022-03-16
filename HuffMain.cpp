#include "Huffman.h"
int main() {
	std::fstream output1(R"(..\HuffmanOrder0\testFile3.txt)", std::ios_base::out | std::ios_base::binary);
	std::fstream input(R"(..\HuffmanOrder0\testFile.txt)", std::ios_base::in | std::ios_base::binary);
	auto output = stl::OpenOutputBitFile(R"(..\HuffmanOrder0\testFile2.txt)");
	auto input1 = stl::OpenInputBitFile(R"(..\HuffmanOrder0\testFile2.txt)");
	
	try {
		std::cout << "compression started....\n";
		compressFile(input, output);
		std::cout << "\nFile compression complete\n";
		stl::closeOutputBitFile(output);
		std::cout << "Expansion started....\n";
		expandFile(input1, output1);
		std::cout << "File expansion complete\n";
		stl::closeInputBitFile(input1);
	}
	catch (stl::FileError const& error) {
		std::cout << error.what();
		std::cout << "File compression failed\n";
	}
	catch (...) {
		std::cout << "An error occurred during compression\n";
	}
}