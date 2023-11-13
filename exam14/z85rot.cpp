#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

template <typename T = uint8_t>
std::vector<T> fromDecToBase(size_t n, const T& base, size_t v_size = 0) {
	std::vector<T> new_base;
	do {
		new_base.push_back(n % base);
		n /= base;
	} while (n > 0);
	for (size_t i = new_base.size(); i < v_size; i++) {
		new_base.push_back(0);
	}
	return new_base;
}

template <typename T = uint8_t>
std::size_t fromBaseToDec(std::vector<T> v_base, const T& base) {
	size_t ret = 0;
	for (size_t i = 0; i < v_base.size(); i++) {
		ret += v_base[i] * size_t(std::pow(base, v_base.size() - 1 - i));
	}
	return ret;
}

using rgb = std::array<uint8_t, 3>;

template<typename T>
class mat {
private:
	size_t rows_, cols_;
	std::vector<T> data_;

public:
	mat(size_t rows = 0, size_t cols = 0) 
		: rows_(rows), cols_(cols), data_(rows_*cols_) {}

	auto& rows() { return rows_; }
	auto& cols() { return cols_; }
	size_t size() { return rows_ * cols_; }
	void resize(size_t r, size_t c) {
		rows_ = r;
		cols_ = c;
		data_.resize(rows_ * cols_);
	}

	char* rawdata() { return reinterpret_cast<char*>(data_.data()); }
	const char* rawdata() const { return reinterpret_cast<const char*>(data_.data()); }
	size_t rawsize() { return size() * sizeof(T); }
};

class ppm {
private: 
	std::ifstream is_;
	size_t width_ = 0, height_ = 0;
	uint16_t maxval_ = 0;	// assumed always 255
	bool good = true;

public:
	ppm(std::string fname) : is_(fname, std::ios::binary) {
		std::string token, magic;
		is_ >> token;
		if (token != "P6") {
			good = false; return;
		}
		is_.get();	// whitespace
		// comment assumed only after magic number
		if (is_.peek() == '#') {
			std::string dump;
			std::getline(is_, dump);
		}
		is_ >> width_ >> height_ >> maxval_;
		is_.get();
	}

	explicit operator bool() { return good; }

	bool read(mat<rgb>& img) {
		img.resize(height_, width_);
		is_.read(img.rawdata(), img.rawsize());
		return true;
	}

	auto& width() { return width_; }
	auto& height() { return height_; }

	static bool write(std::string fname, mat<rgb>& img) {
		std::ofstream os(fname, std::ios::binary);
		if (!os) {
			return false;
		}

		os << "P6\n";
		os << img.cols() << " " << img.rows() << "\n";
		os << "255\n";

		os.write(img.rawdata(), img.rawsize());

		return true;
	}
};

void syntax() {
	std::cout << "Z85rot {c | d} <N> <input file> <output file>";
}

template <typename T>
T switch_endianness(const T& val, const size_t n = sizeof(T)) {
	T ret = 0;
	for (size_t i = 0; i < n; i++) {
		ret |= (((val >> 8 * (n - 1 - i)) & 0xFF) << (8 * i));
	}
	return ret;
}

static std::string base85_chars = 
"0123456789"
"abcdefghij"
"klmnopqrst"
"uvwxyzABCD"
"EFGHIJKLMN"
"OPQRSTUVWX"
"YZ.-:+=^!/"
"*?&<>()[]{"
"}@%$#";

template <typename T>
T rotate_uint(T val, T upper_bound, size_t rot, int8_t sign = 0) {
	if (sign >= 0) {
		rot %= upper_bound;
	}
	else {
		rot = upper_bound - rot % upper_bound;
	}
	return (val + rot) % upper_bound;
}

/*
c 1 2x2red.ppm red.z85
d 1 red.z85 red.dec.ppm

c 1 rana.ppm rana.z85
d 1 rana.z85 rana.dec.ppm

c 13 landscape.ppm landscape.z85
d 13 landscape.z85 landscape.dec.ppm
*/

int main(int argc, char** argv) {
	if (argc != 5) {
		syntax();
		return EXIT_FAILURE;
	}

	std::string cmd = argv[1];
	unsigned N = std::stoi(argv[2]);
	std::string srcfname = argv[3];
	std::string outfname = argv[4];

	if (cmd == "c") {
		mat<rgb> img;
		ppm ppm_(srcfname);
		if (ppm_) ppm_.read(img);
		else return EXIT_FAILURE;

		std::ofstream os(outfname, std::ios::binary);
		os << ppm_.width() << "," << ppm_.height() << ",";

		uint32_t _4_bytes;
		size_t rcount = 0;
		for (size_t i = 0; i < img.rawsize(); i += 4) {
			_4_bytes = *reinterpret_cast<uint32_t*>(&img.rawdata()[i]);
			_4_bytes = switch_endianness(_4_bytes);
			std::vector<uint8_t> b85 = fromDecToBase(_4_bytes, uint8_t(85), 5);
			for (size_t j = 0; j < b85.size(); j++) {
				uint8_t r = rotate_uint(b85[b85.size() - 1 - j], uint8_t(85), N * rcount++, int8_t(-1));
				os << base85_chars[r];
			}
		}
	}
	else if (cmd == "d") {
		std::ifstream is(srcfname, std::ios::binary);
		if (!is) {
			return EXIT_FAILURE;
		}

		size_t width, height;
		is >> width;
		is.get();
		is >> height;
		is.get();
		mat<rgb> img(height, width);

		char cquintet[5];
		std::vector<uint8_t> uquintet(5);
		size_t rcount = 0;
		size_t out_bytes = 0;
		while (is.read(cquintet, 5)) {
			for (uint8_t i = 0; i < 5; i++) {
				uquintet[i] = base85_chars.find(cquintet[i]);
				uquintet[i] = rotate_uint(uquintet[i], uint8_t(85), N * rcount++);
			}
			uint32_t n = uint32_t(fromBaseToDec(uquintet, uint8_t(85)));
			n = switch_endianness(n);
			std::copy_n(reinterpret_cast<char*>(&n), 4, &img.rawdata()[out_bytes]);
			out_bytes += 4;
		}

		ppm::write(outfname, img);
	}

	return EXIT_SUCCESS;
}
