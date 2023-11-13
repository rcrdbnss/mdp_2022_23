#include "hufstr.h"
#include <sstream>

hufstr::hufstr() {
	std::fstream is("table.bin", std::ios::binary | std::ios::in);
	std::string header;
	is >> header;
	if (header != "HUFFMAN") {
		std::cerr << "Error 1";
		return;
	}
	while (true) {
		elem<uint8_t> e;
		is.read(reinterpret_cast<char*>(&e._sym), 1);
		if (!is) {
			break;
		}
		is.read(reinterpret_cast<char*>(&e._len), 1);
		is.read(reinterpret_cast<char*>(&e._code), 4);
		sym_table_.insert({ e._sym, e });
		code_table_.insert({ std::make_pair(e._len, e._code), e._sym });
	}
}

std::vector<uint8_t> hufstr::compress(const std::string& s) const {
	std::ostringstream os;
	BitWriter bw(os);

	for (size_t i = 0; i < s.size(); i++) {
		uint8_t ch = s[i];
		elem<uint8_t> e = sym_table_.at(ch);
		bw.write(e._code, e._len);
	}
	bw.flush(1);

	std::string str = os.str();
	std::vector<uint8_t> v(str.begin(), str.end());
	return v;
}

std::string hufstr::decompress(const std::vector<uint8_t>& v) const {
	std::stringstream out;
	uint32_t code = 0;
	uint8_t len = 0;
	for (size_t i = 0; i < v.size(); i++) {
		uint8_t byte = v[i];
		for (int b = 0; b < 8; b++) {
			code = (code << 1) | ((byte >> (7 - b) & 1));
			len++;
			auto p = std::make_pair(len, code);
			if (code_table_.count(p)) {
				out << code_table_.at(p);
				code = 0;
				len = 0;
			}
		}
	}
	return out.str();
}
