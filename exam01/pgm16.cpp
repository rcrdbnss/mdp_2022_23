#include <fstream>
#include <sstream>
#include <iostream>

#include "pgm16.h"

template<typename T>
void switch_endiannes(T& in, int n = -1) {
	T out = 0;
	if (n == -1) {
		n = sizeof(T);
	}
	for (int i = 0; i < n; i++) {
		out = out | ((in >> (8 * (n - i - 1)) & 255) << (8 * i));
	}
	in = out;
}

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string magic_number;
	is >> magic_number;
	if (magic_number != "P5") {
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
	int width, height;
	is >> width;
	is >> height;
	is >> maxvalue;

	c = is.get();
	if (c != '\n') {
		return false;
	}

	img.resize(height, width);

	for (int h = 0; h < height; h++) {
		for (int w = 0; w < width; w++) {
			if (maxvalue < 256) {
				uint8_t byte = 0;
				is.read(reinterpret_cast<char*>(&byte), 1);
				img(h, w) = byte;
			}
			else {
				uint16_t bytes = 0;
				is.read(reinterpret_cast<char*>(&bytes), 2);	// big endian
				switch_endiannes<uint16_t>(bytes);	// little endian, suited for C++
				img(h, w) = bytes;
			}
		}
	}
	
	return true;
}
