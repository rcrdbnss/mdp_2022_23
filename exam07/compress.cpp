// #include "compress.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include "mat.h"

int run(const mat<uint8_t>& img, int i_first_byte, std::vector<uint8_t>& encoded) {
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

int copy(const mat<uint8_t>& img, int i_first_byte, std::vector<uint8_t>& encoded) {
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

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	int len = 0;
	for (int i = 0; i + 1 < img.size(); i += len) {
		uint8_t pre = img.data()[i];
		uint8_t cur = img.data()[i + 1];
		if (cur == pre) {
			len = run(img, i, encoded);
		}
		else {
			len = copy(img, i, encoded);
		}
	}
	encoded.push_back(128);
}

static std::string base64_chars = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::string Base64Encode(const std::vector<uint8_t>& v) {
	uint32_t buffer = 0;
	std::stringstream out;
	for (size_t i = 0; i < v.size(); i += 3) {
		uint8_t j = 0;
		for (; j < 3 && i + j < v.size(); j++) {
			buffer = buffer << 8 | v[i + j];
		}
		for (; j < 3; j++) {
			buffer = buffer << 8 | 128;
		}

		for (uint8_t k = 0; k < 4; k++) {
			unsigned char c = (buffer >> (6 * (3 - k))) & 0x3f;
			out << base64_chars[c];
		}
	}

	return out.str();
}
