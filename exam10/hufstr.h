#pragma once
#include <iostream> 
#include <fstream>
#include <string>
//#include <unordered_map>
#include <map>
#include <utility>
#include <vector>

template<typename T>
struct elem {
	T _sym;
	uint8_t _len = 0;
	uint32_t _code = 0;
	elem() {}
	elem(const T& sym) : _sym(sym) {}
	bool operator<(const elem& rhs) const {
		if (_len < rhs._len)
			return true;
		else if (_len > rhs._len)
			return false;
		else
			return _sym < rhs._sym;
	}
};

typedef std::pair<uint8_t, uint32_t> pair;

class hufstr {
	/*std::unordered_map<uint8_t, elem<uint8_t>> sym_table_;
	std::unordered_map<pair, elem<uint8_t>> code_table_;*/
	std::map<uint8_t, elem<uint8_t>> sym_table_;
	std::map<pair, uint8_t> code_table_;
public:
    hufstr();
    std::vector<uint8_t> compress(const std::string& s) const;
    std::string decompress(const std::vector<uint8_t>& v) const;
};

class BitWriter {
	uint8_t buffer_ = 0;
	uint8_t nbits_ = 0;
	std::ostream& os_;

public:
	BitWriter(std::ostream& os) : os_(os) {}

	~BitWriter() { flush(); }

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

	template <typename T>
	std::ostream& write(T val, uint8_t n) {
		while (n-- > 0) {
			uint8_t bit = (val >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}
};
