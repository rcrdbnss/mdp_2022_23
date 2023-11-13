#include "process_ppm.h"
#include <fstream>
#include "ppm.h"

template <typename T>
void switch_endianness(T& in, size_t n = sizeof(T)) {
	T out = 0;
	for (size_t i = 0; i < n; i++) {
		out = (out << 8) | ((in >> (8 * (n - i - 1))) & 0xff);
	}
	in = out;
}

bool LoadPPM(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	std::string magicn;
	is >> magicn;
	if (magicn != "P6") {
		return false;
	}

	char ch;
	std::string numbers = "0123456789";
	while (true) {
		ch = is.peek();
		if (ch == '#') {
			std::string dump;
			std::getline(is, dump);
		}
		else if (numbers.find(ch) == std::string::npos) {
			is.get(ch);
		}
		else {
			break;
		}
	}

	int width, height;
	uint16_t maxval;
	is >> width >> height >> maxval;

	img.resize(height, width);
	uint8_t bytes = maxval < 256 ? 1 : 2;

	is.get(ch); // single whitespace, ignore

	for (int r = 0; r < height; r++) {
		for (int c = 0; c < width; c++) {
			// is >> img(r, c); // skips whitespace bytes, such as 0A 
			for (size_t i = 0; i < 3; i++) {
				uint16_t x;
				is.read(reinterpret_cast<char*>(&x), bytes);
				if (bytes == 2) switch_endianness(x, bytes);
				img(r, c)[i] = x;
			}
		}
	}

	return true;
}

void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
	img_r.resize(img.rows(), img.cols());
	img_g.resize(img.rows(), img.cols());
	img_b.resize(img.rows(), img.cols());

	for (int r = 0; r < img.rows(); r++) {
		for (int c = 0; c < img.cols(); c++) {
			img_r(r, c) = img(r, c)[0];
			img_g(r, c) = img(r, c)[1];
			img_b(r, c) = img(r, c)[2];
		}
	}
	return;
}
