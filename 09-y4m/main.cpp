#include <iostream>
#include "mat.h"
#include "y4m_gray.h"
#include "y4m_color.h"

int main(int argc, char** argv) {
	{
		std::string srcfname = argv[1];
		std::vector<mat<uint8_t>> frames;
		std::vector<mat<vec3b>> frames3b;
		// 720p_stockholm_ter.y4m
		// foreman_cif.y4m
		// test1.y4m
		bool ret = y4m_extract_color(srcfname, frames3b);
		if (!ret) return EXIT_FAILURE;
	}
	_CrtDumpMemoryLeaks();
	return EXIT_SUCCESS;
}
