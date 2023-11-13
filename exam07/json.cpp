// #include "json.h"
#include <fstream>
#include <sstream>
#include <string>
#include "mat.h"
#include "ppm.h"

template <typename T>
void switch_endianness_(T& in, size_t n = sizeof(T)) {
	T out = 0;
	for (size_t i = 0; i < n; i++) {
		out = (out << 8) | ((in >> (8 * (n - i - 1))) & 0xff);
	}
	in = out;
}

bool LoadPPM_(const std::string& filename, mat<vec3b>& img) {
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
				if (bytes == 2) switch_endianness_(x, bytes);
				img(r, c)[i] = x;
			}
		}
	}

	return true;
}

void SplitRGB_(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
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

int run_(const mat<uint8_t>& img, int i_first_byte, std::vector<uint8_t>& encoded) {
	uint8_t run_len = 255;
	uint8_t run_val = img.data()[i_first_byte];
	for (int i = i_first_byte + 1;
		i + 1 < img.size() && img.data()[i] == img.data()[i + 1] && run_len > 128;
		i++) {
		run_len--;
	}
	encoded.push_back(run_len);
	encoded.push_back(run_val);
	return 257 - run_len;
}

int copy_(const mat<uint8_t>& img, int i_first_byte, std::vector<uint8_t>& encoded) {
	uint8_t cpy_len = 0;
	int i;
	for (i = i_first_byte + 1;
		i + 1 < img.size() && img.data()[i] != img.data()[i + 1] && cpy_len < 128;
		i++) {
		cpy_len++;
	}
	if (i + 1 == img.size()) {	// last element
		cpy_len++;
	}
	encoded.push_back(cpy_len);
	std::copy_n(
		std::next(img.data(), i_first_byte), cpy_len + 1, std::back_inserter(encoded)
	);
	return cpy_len + 1;
}

void PackBitsEncode_(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	int len = 0;
	for (int i = 0; i + 1 < img.size(); i += len) {
		uint8_t pre = img.data()[i];
		uint8_t cur = img.data()[i + 1];
		if (cur == pre) {
			len = run_(img, i, encoded);
		}
		else {
			len = copy_(img, i, encoded);
		}
	}
	encoded.push_back(128);
}

static std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string Base64Encode_(const std::vector<uint8_t>& v) {
	uint32_t buffer;
	std::stringstream out;
	for (size_t i = 0; i < v.size(); i += 3) {
		buffer = 0;
		uint8_t j = 0;
		for (; j < 3 && i + j < v.size(); j++) {
			buffer = buffer << 8 | v[i + j];
		}
		for (; j < 3; j++) {
			buffer = buffer << 8 | 0x80;
		}

		for (uint8_t k = 0; k < 4; k++) {
			unsigned char c = (buffer >> (6 * (3 - k))) & 0x3f;
			out << base64_chars[c];
		}
	}

	return out.str();
}

std::string JSON(const std::string& filename) {
	mat<vec3b> img;
	if (!LoadPPM_(filename, img)) {
		return "{}";
	}

	mat<uint8_t> img_r; mat<uint8_t> img_g; mat<uint8_t> img_b;
	SplitRGB_(img, img_r, img_g, img_b);

	std::vector<uint8_t> r, g, b;
	PackBitsEncode_(img_r, r);
	PackBitsEncode_(img_g, g);
	PackBitsEncode_(img_b, b);

	std::string b64r, b64g, b64b;
	b64r = Base64Encode_(r);
	b64g = Base64Encode_(g);
	b64b = Base64Encode_(b);

	std::stringstream json;
	json << "{";
	json << "\n\t" "\"width\": " << img.cols();
	json << ",\n\t" "\"height\": " << img.rows();
	json << ",\n\t" "\"red\": \"" << b64r << "\"";
	json << ",\n\t" "\"green\": \"" << b64g << "\"";
	json << ",\n\t" "\"blue\": \"" << b64b << "\"";
	json << "\n}";

	return json.str();
}
