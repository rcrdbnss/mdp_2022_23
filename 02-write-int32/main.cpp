#include <fstream>
#include <iostream>
#include <vector>

void syntax() {
	std::cout
		<< "SYNTAX:\n"
		<< "write_int32 <filesrc.txt> <fileout.bin>\n"
		<< "Writes numbers as 32 bits little endian integers.\n";
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << '\n';
	exit(EXIT_FAILURE);
}

class BitWriter {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	void write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		// when the buffer is full, write it
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
	}

public:
	BitWriter(std::ostream& os) : os_(os) {}

	~BitWriter() {
		flush();
	}

	std::ostream& write(uint32_t u, uint8_t n) {
		while (n-- > 0) {
			uint8_t bit = (u >> n) & 1;
			write_bit(bit);
		}
		return os_;
	}

	std::ostream& operator() (uint32_t u, uint8_t n) {
		return write(u, n);
	}

	void flush(uint8_t bit = 0) {
		while (nbits_ > 0) {
			write_bit(bit);
		}
	}

	template<typename T>
	static std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
		return os.write(reinterpret_cast<const char*>(&num), size);
	}
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		syntax();
	}

	std::string srcfname = argv[1];
	std::string outfname = argv[2];

	std::ifstream is(srcfname);
	if (!is) {
		error("Cannot open file " + srcfname);
	}

	std::vector<int32_t> v{
		std::istream_iterator<int32_t>(is),
		std::istream_iterator<int32_t>()
	};

	std::ofstream os(outfname, std::ios::binary);
	if (!os) {
		error("Cannot open file " + outfname);
	}

	BitWriter bw(os);
	for (const auto& x : v) {
		bw(x, 11);
	}

	return EXIT_SUCCESS;
}
