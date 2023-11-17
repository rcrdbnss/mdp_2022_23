#include <iostream>
#include <fstream>
#include <vector>

void syntax() {
	std::cout
		<< "SYNTAX:\n\n"
		<< "read_int32 <filesrc.bin> <fileout.txt>\n"
		<< "Reads 32 bits little endian integers and outputs their textual decimal representation\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << "\n";
	exit(EXIT_FAILURE);
}

class BitReader {
private:
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::istream& is_;

public:
	BitReader(std::istream& is) : is_(is) {}

	int read_bit() {
		if (nbits_ == 0) {
			if (!raw_read(is_, buffer_)) {
				return EOF;
			}
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	explicit operator bool() { return bool(is_); }
	bool operator!() { return !is_; }

	template <typename T>
	std::istream& read(T& u, size_t n) {
		u = 0;
		while (n --> 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	template<typename T>
	static std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
		return is.read(reinterpret_cast<char*>(&val), size);
	}
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		syntax();
	}
	std::string srcfname = argv[1];
	std::string outfname = argv[2];

	std::ifstream is(srcfname, std::ios::binary);
	if (!is) {
		error("Cannot open file " + srcfname);
	}

	std::vector<int32_t> v;
	BitReader br(is);
	uint32_t u;
	while (br.read(u, 11)) {
		if (u > 1023) {
			u -= 2048;
		}
		v.push_back(u);
	}

	std::ofstream os(outfname);
	if (!os) {
		error("Cannot open file " + outfname);
	}

	for (const auto& x : v) {
		os << x << '\n';
	}

	return EXIT_SUCCESS;
}
