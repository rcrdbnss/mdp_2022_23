#include <cmath>
#include <fstream>
#include <map>
#include <vector>

//#include "lz78encode.h"
bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits);

//int main() {
//	{
//		lz78encode("bibbia.txt", "bibbia10.bin", 10);
//	}
//	_CrtDumpMemoryLeaks();
//	return EXIT_SUCCESS;
//}

class BitWriter {
private:
	uint8_t buffer_ = 0;
	uint8_t n_bits_ = 0;
	std::ostream& os_;

public:
	BitWriter(std::ostream& os) : os_(os) {}

	~BitWriter() { flush(); }

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		n_bits_++;
		if (n_bits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			n_bits_ = 0;
		}
	}

	template <typename T>
	std::ostream& write(T arg, uint8_t n) {
		while (n-- > 0) {
			uint8_t bit = (arg >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	template <typename T>
	std::ostream operator() (T arg, uint8_t n) {
		return write(arg, n);
	}

	void flush(uint8_t bit = 0) {
		while (n_bits_ > 0) {
			write_bit(bit);
		}
	}
};

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {
	std::ifstream is(input_filename, std::ios::binary);
	if (!is) {
		return false;
	}
	std::ofstream os(output_filename, std::ios::binary);
	if (!os) {
		return false;
	}
	if (!(1 <= maxbits && maxbits <= 30)) {
		return false;
	}
	uint32_t dict_max_size = uint32_t(pow(2, maxbits));

	BitWriter bw(os);
	os << "LZ78";
	bw.write(maxbits, 5);

	std::map<std::string, uint32_t> dict;
	std::string pre = "";
	char ch;
	uint8_t n_bits = 0;
	// read, check, use
	while(is.read(&ch, 1)) {
		if (dict.count(pre + ch)) {
			pre += ch;
		}
		else {
			uint32_t dict_ref = pre.empty() ? 0 : dict[pre];
			if (!dict.empty()) {
				n_bits = uint8_t(floor(log2(dict.size()))) + 1;
				bw.write(dict_ref, n_bits);
			}
			bw.write(ch, 8);

			dict[pre + ch] = uint32_t(dict.size() + 1);
			if (dict.size() + 1 > dict_max_size) {
				dict.clear();
				n_bits = 0;
			}
			pre.clear();
		}
	}
	if (!pre.empty()) {
		ch = pre.back();
		pre = pre.substr(0, pre.size() - 1);

		uint32_t dict_ref = pre.empty() ? 0 : dict[pre];
		if (!dict.empty()) {
			n_bits = uint8_t(floor(log2(dict.size()))) + 1;
			bw.write(dict_ref, n_bits);
		}
		bw.write(ch, 8);
	}
	
	bw.flush();
	return true;
}
