#include <cstdlib>
#include <fstream>
#include "lzs.h"

int main() {
	std::string srcfname;
	// srcfname = "test.txt.lzs";
	// srcfname = "prova.txt.lzs";
	srcfname = "bibbia.txt.lzs";
	std::string outfname = srcfname + ".txt";
	std::ifstream is(srcfname, std::ios::binary);
	std::ofstream os(outfname, std::ios::binary);
	lzs_decompress(is, os);
	return EXIT_SUCCESS;
}