#include <sstream>
#include <cmath>
#include <algorithm>
#include "base64.h"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static char base64_index(char c) {
	size_t found = base64_chars.find(c);
	return found == std::string::npos ? -1 : char(found);
}

std::string base64_decode(const std::string& input) {
	std::stringstream in, out;
	// copy the input string into a sstream, removing whitespaces and newlines
	std::copy_if(
		input.begin(), input.end(), std::ostreambuf_iterator<char>(in),
		[](const auto& c) {
			return base64_index(c) >= 0 || c == '=';
		});

	char chars[4];
	uint32_t buffer;
	size_t count;	// sextets
	while (in.read(chars, 4)) {
		buffer = 0;
		count = 0;
		for (size_t i = 0; i < 4; i++) {
			char c = base64_index(chars[i]);	// fetch sextet
			if (c >= 0) {
				buffer = buffer << 6 | (c & 0x3f);
				count++;
			}
			else {
				buffer <<= 6;
			}
		}
		/*
		Special processing is performed if fewer than 24 bits are available
		at the end of the data being encoded. A full encoding quantum is
		always completed at the end of a body. When fewer than 24 input bits
		are available in an input group, zero bits are added (on the right)
		to form an integral number of 6-bit groups. Padding at the end of
		the data is performed using the '=' character. Since all base64
		input is an integral number of octets, only the following cases can
		arise: 
		(1) the final quantum of encoding input is an integral multiple of 
		24 bits; here, the final unit of encoded output will be an integral 
		multiple of 4 characters with no "=" padding, 
		(2) the final quantum of encoding input is exactly 8 bits; here, the
		final unit of encoded output will be two characters followed by two 
		"=" padding characters, or 
		(3) the final quantum of encoding input is exactly 16 bits; here, the
		final unit of encoded output will be three characters followed by one
		"=" padding character.
		*/
		// the number of octets is always the number of sextets - 1
		for (size_t j = 0; j < count - 1; j++) {
			char c = (buffer >> (8 * (2 - j)));
			out << c;
		}
	}

	return out.str();
}

std::string base64_encode(const std::string& input) {
	std::stringstream in, out;
	std::copy(input.begin(), input.end(), std::ostreambuf_iterator<char>(in));

	char octets[3] = { 0, 0, 0 };
	// while (in.read(octets, 3)) does not work, because it goes false if less than 3 bytes are available. Must check is after the execution
	do 
	{
		// in.read(octets, 3);
		size_t count = in.gcount();
		if (count > 0) {
			uint32_t buffer = 0;
			size_t i = 0;
			for (; i < count; i++) {
				buffer = buffer << 8 | octets[i];
			}
			for (; i < 3; i++) {
				buffer = buffer << 8;
			}
			// the number of sextets is always octets + 1
			size_t j = 0;
			for (; j < count + 1; j++) {
				unsigned char c = (buffer >> (6 * (3 - j))) & 0x3f;
				out << base64_chars[c];
			}
			for (; j < 4; j++) {
				out << '=';
			}
		}
	} 
	while (in);

	return out.str();
}
