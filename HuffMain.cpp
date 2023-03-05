#include "Huffman.h"
#include <chrono>
#include <filesystem>
#include <format>
namespace fs = std::filesystem;

struct Timer {
public:
	Timer() = default;
	void Start() {
		start = std::chrono::high_resolution_clock::now();
	}
	void Stop() {
		stop = std::chrono::high_resolution_clock::now();
	}
	float time() {
		elapsedTime = std::chrono::duration<float>(stop - start).count();
		return elapsedTime;
	}
private:
	float elapsedTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> start, stop;
};

float fileSize(fs::path const& path) {
	auto lengthInKb = fs::file_size(path);
	return lengthInKb;
}

int main() {
	auto timer = Timer();
	try {
		std::fstream input(R"(..\HuffmanOrder0\testFile.txt)", std::ios_base::in | std::ios_base::binary);
		auto output = stl::OpenOutputBitFile(R"(..\HuffmanOrder0\testFile2.txt)");
		std::cout << "compression started....\n";
		timer.Start();
		compressFile(input, output);
		timer.Stop();
		std::cout << "\nFile compression complete\n";
		printf("Normal Huffman compression time = %f seconds\n\n", timer.time());
		stl::closeOutputBitFile(output);


		std::fstream output1(R"(..\HuffmanOrder0\testFile3.txt)", std::ios_base::out | std::ios_base::binary);
		auto input1 = stl::OpenInputBitFile(R"(..\HuffmanOrder0\testFile2.txt)");
		std::cout << "Expansion started....\n";
		timer.Start();
		expandFile(input1, output1);
		timer.Stop();
		std::cout << "\nFile expansion complete\n";
		printf("Normal Huffman expansion time = %f seconds\n\n", timer.time());
		stl::closeInputBitFile(input1);
		output1.close();

		std::cout << std::format("Original file size = {} bytes\n", fileSize(fs::path(R"(..\HuffmanOrder0\testFile.txt)")));
		std::cout << std::format("Compressed file size = {} bytes\n", fileSize(fs::path(R"(..\HuffmanOrder0\testFile2.txt)")));
		std::cout << std::format("Expanded file size = {} bytes\n", fileSize(fs::path(R"(..\HuffmanOrder0\testFile3.txt)")));
	}
	catch (stl::FileError const& error) {
		std::cout << error.what();
		std::cout << "File compression failed\n"; 
	}
	catch (...) {
		std::cout << "An error occurred during compression\n";
	}
}