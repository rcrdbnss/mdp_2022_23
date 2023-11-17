#include <cstdlib>
#include <iostream>
#include <fstream>
#include <array>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "frequency.h"

void syntax() {
	std::cout
		<< "SYNTAX:\n"
		<< "frequency <input file> <output file>\n"
		<< "The program computes the frequency of every byte in the input file\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << "\n";
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {

	if (argc != 3) {
		syntax();
	}

	std::string srcfname = argv[1];
	std::string outfname = argv[2];

	std::cout << "Reading file and computing frequencies...";

	std::ifstream is(srcfname, std::ios::binary);
	if (!is) {
		error("Cannot open file " + srcfname);
	}
	is.unsetf(std::ios::skipws);

	FrequencyCounter<uint8_t> f;
	f = std::for_each(std::istream_iterator<uint8_t>{is},
		std::istream_iterator<uint8_t>{}, f);

	std::cout << " done.\n";

	std::cout << "Saving...";

	std::ofstream os(outfname);
	if (!os) {
		error("Cannot open file " + outfname);
	}

	for (const auto& x: f) {
		os
			<< std::hex << std::setw(2) << std::setfill('0')
			<< +x.first
			<< std::dec
			<< '\t' << x.second << '\n';
		os.flush();
	}

	std::cout << " done.\n";

	return EXIT_SUCCESS;
}
