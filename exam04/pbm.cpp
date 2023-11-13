#include <fstream>
#include <iostream>
#include <cmath>
#include "pbm.h"

bool BinaryImage::ReadFromPBM(const std::string& filename) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string magic_number;
	is >> magic_number;
	if (magic_number != "P4") {
		return false;
	}

	char c;
	is.get(c);
	if (c != '\n') {
		return false;
	}

	c = is.peek();
	if (c == '#') { // comment
		std::string dump;
		std::getline(is, dump);
	}

	is >> W >> H;

	c = is.get();
	if (c != '\n') {
		return false;
	}

	std::copy(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), 
		std::back_inserter(ImageData));

	return true;
}

Image BinaryImageToImage(const BinaryImage& bimg) {
	Image img;
	img.W = bimg.W;	// bits
	img.H = bimg.H;
	img.ImageData = std::vector<uint8_t>(img.W * img.H);

	int Wbytes = int(std::ceil(img.W / 8.));

	for (int r = 0; r < img.H; r++) {
		for (int c = 0; c < Wbytes; c++) {
			const uint8_t byte = bimg.ImageData[r * Wbytes + c];
			for (int i = 0; i < 8 && (c * 8 + i) < img.W; i++) {
				const uint8_t bit = (byte >> (7 - i)) & 0x01;
				img.ImageData[r * img.W + c * 8 + i] = bit ? 0x00 : 0xff;
			}
		}
	}

	return img;
}
