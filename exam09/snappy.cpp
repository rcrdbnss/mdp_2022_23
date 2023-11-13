#include <fstream>
#include <iostream>
#include <string>

void snappy_decode(std::string srcfile, std::string outfile);
size_t read_varint(std::ifstream& is);
uint8_t tag_type(uint8_t tag);
size_t read_literal(uint8_t tag, std::ifstream& is, std::fstream& os);
size_t copy(uint8_t tag, std::ifstream& is, std::fstream& os);

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Syntax error.";
	}

	std::string srcfname = argv[1];
	std::string outfname = argv[2];
	
	snappy_decode(srcfname, outfname);

	return EXIT_SUCCESS;
}

void snappy_decode(std::string srcfile, std::string outfile) {
	std::ifstream is(srcfile, std::ios::binary);
	if (!is) {
		std::cerr << "Error 1";
		return;
	}

	std::fstream os(outfile,
		std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
	if (!os) {
		std::cerr << "Error 2";
		return;
	}

	size_t size = read_varint(is);
	size_t size_count = 0;

	uint8_t tag = 0;
	is.read(reinterpret_cast<char*>(&tag), 1);
	if (tag_type(tag) != 0) {
		std::cerr << "Error 3";
		return;
	}
	size_count += read_literal(tag, is, os);

	while (is.read(reinterpret_cast<char*>(&tag), 1)) {
		if (tag_type(tag) == 0) {
			size_count += read_literal(tag, is, os);
		}
		else {
			size_count += copy(tag, is, os);
		}
	}

	if (size != size_count) {
		std::cerr << "Errore 4";
	}
}

size_t read_varint(std::ifstream& is) {
	size_t val = 0;
	size_t byte = 0;
	uint8_t i = 0;

	while (is.read(reinterpret_cast<char*>(&byte), 1)) {
		val = val | ((byte & 0x7F) << (7 * i++));
		if (((byte >> 7) & 1) == 0) {
			break;
		}
	}
	return val;
}

uint8_t tag_type(uint8_t tag) {
	return tag &= 0x03;
}

size_t read_literal(uint8_t tag, std::ifstream& is, std::fstream& os) {
	uint8_t msb6 = (tag >> 2) & 0x3F;
	uint32_t len = 0;
	if (msb6 < 60) {
		len = msb6 + 1;
	}
	else {
		uint8_t to_read = msb6 - 59;
		is.read(reinterpret_cast<char*>(&len), to_read);
		len++;
	}
	std::string s;
	s.resize(len);
	is.read(reinterpret_cast<char*>(&s[0]), len);
	os.write(reinterpret_cast<char*>(&s[0]), len).flush();
	return size_t(len);
}

size_t copy(uint8_t tag, std::ifstream& is, std::fstream& os) {
	uint8_t len = 0;
	uint32_t offset = 0;
	char byte = 0;
	switch (tag & 0x03) {
	case 1:
		len = (tag >> 2 & 0x07) + 4;
		offset = (tag >> 5) & 0x07;
		is.read(&byte, 1);
		offset = (offset << 8) | uint8_t(byte); // signed char totally scrambles it!!!
		break;

	case 2:
		len = (tag >> 2 & 0x3F) + 1;
		is.read(reinterpret_cast<char*>(&offset), 2);
		break;

	case 3:
		len = (tag >> 2 & 0x3F) + 1;
		is.read(reinterpret_cast<char*>(&offset), 4);
		break;

	default:
		break;
	}

	uint8_t to_copy = len;
	while (to_copy > 0) {
		uint8_t len_ = to_copy > offset ? offset : to_copy;
		std::string s;
		s.resize(len_);
		auto pos = os.tellg();
		os.seekg(-int64_t(offset), os.end);
		os.read(reinterpret_cast<char*>(&s[0]), len_);
		os.seekp(pos);
		auto isg = is.tellg();
		os.write(reinterpret_cast<char*>(&s[0]), len_).flush();
		to_copy -= len_;
		s.clear();
	}

	return size_t(len);
}
