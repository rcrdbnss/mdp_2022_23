#include <cstdlib>
#include "pgm16.h"

int main(void) {
	std::string filename;
	// @mem(&img.data_[0], UINT8, 2, img.cols_, img.rows_, img.cols_*2)
	//filename = "frog_bin.pgm";
	filename = "CR-MONO1-10-chest.pgm";
	mat<uint16_t> img;
	uint16_t maxvalue;

	if (load(filename, img, maxvalue)) {
		return EXIT_SUCCESS;
	}
	else {
		return EXIT_FAILURE;
	}
}