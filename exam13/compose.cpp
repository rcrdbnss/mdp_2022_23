#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using rgba = std::array<uint8_t, 4>;

template <typename T>
class mat {
	size_t cols_, rows_;
	std::vector<T> data_;

public:
	mat(size_t rows = 0, size_t cols = 0) 
		: rows_(rows), cols_(cols), data_(rows * cols) {}

	T& operator[](size_t i) { return data_[i]; }
	const T& operator[](size_t i) const { return data_[i]; }
	T& operator()(size_t r, size_t c) { return data_[r * cols_ + c]; }
	const T& operator()(size_t r, size_t c) const { return data_[r * cols_ + c]; }

	void resize(size_t rows, size_t cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows_ * cols_);
	}

	auto rows() const { return rows_; }
	auto cols() const { return cols_; }
	auto size() const { return rows_ * cols_; }
	auto& data() { return data_; }
	const auto& data() const { return data_; }
	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }

	char* rawdata() { return reinterpret_cast<char*>(data_.data()); }
	const char* rawdata() const { return reinterpret_cast<const char*>(data_.data()); }
	auto rawsize() const { return rows_ * cols_ * sizeof(T); }
};

class pam {
public:
	static bool read_header(const std::string& fname, mat<rgba>& img) {
		std::ifstream is(fname, std::ios::binary);
		if (!is) {
			return false;
		}

		std::string magic;
		is >> magic;
		if (magic != "P7") {
			return false;
		}

		std::string token;
		size_t width, height, depth, maxval;
		std::string tupltype;
		while (is >> token) {
			if (token == "WIDTH") {
				is >> width;
			}
			else if (token == "HEIGHT") {
				is >> height;
			}
			else if (token == "DEPTH") {
				is >> depth;
			}
			else if (token == "MAXVAL") {
				is >> maxval;
			}
			else if (token == "TUPLTYPE") {
				is >> tupltype;
			}
			else if (token == "ENDHDR") {
				if (is.get() != '\n') {
					return false;
				}
				break;
			}
			else {
				std::string dump;
				std::getline(is, dump);
			}
		}
		img.resize(height, width);
		return true;
	}

	static bool read(const std::string& fname, mat<rgba>& img) {
		std::ifstream is(fname, std::ios::binary);
		if (!is) {
			return false;
		}

		std::string magic;
		is >> magic;
		if (magic != "P7") {
			return false;
		}

		std::string token;
		size_t width, height, depth, maxval;
		std::string tupltype;
		while (is >> token) {
			if (token == "WIDTH") {
				is >> width;
			}
			else if (token == "HEIGHT") {
				is >> height;
			}
			else if (token == "DEPTH") {
				is >> depth;
			}
			else if (token == "MAXVAL") {
				is >> maxval;
			}
			else if (token == "TUPLTYPE") {
				is >> tupltype;
			}
			else if (token == "ENDHDR") {
				if (is.get() != '\n') {
					return false;
				}
				break;
			}
			else {
				std::string dump;
				std::getline(is, dump);
			}
		}

		img.resize(height, width);
		if (depth == 4 && tupltype == "RGB_ALPHA") {
			is.read(img.rawdata(), img.rawsize());
		}
		else if (depth == 3 && tupltype == "RGB") {
			for (size_t i = 0; i < img.size(); i++) {
				rgba pixel; 
				is.read(reinterpret_cast<char*>(&pixel), 3);
				pixel[3] = 255;
				img[i] = pixel;
			}
		}
		return is.good();
	}

	static bool write(const std::string& fname, mat<rgba>& img) {
		std::ofstream os(fname, std::ios::binary);
		if (!os) {
			return false;
		}

		os << "P7\n";
		os << "WIDTH " << img.cols() << "\n";
		os << "HEIGHT " << img.rows() << "\n";
		os << "DEPTH 4\n";
		os << "MAXVAL 255\n";
		os << "TUPLTYPE RGB_ALPHA\n";
		os << "ENDHDR\n";

		os.write(img.rawdata(), img.rawsize());
		return true;
	}
};

struct srcimg {
	size_t x_ = 0;
	size_t y_ = 0;
	std::string name_;
};

rgba compose_pixel(rgba& pA, rgba& pB) {
	rgba pO = { 0 };
	using d = double;
	d aA = d(pA[3]) / 255.;
	d aB = d(pB[3]) / 255.;
	d aO = aA + aB * (1. - aA);
	for (uint8_t j = 0; j < 3; j++) {
		pO[j] = uint8_t(std::round((d(pA[j]) * aA + d(pB[j]) * aB * (1. - aA)) / aO));
	}
	pO[3] = uint8_t(aO * 255);
	return pO;
}

int main(int argc, char** argv) {
	//{
		std::string outimgname = std::string(argv[1]) + ".pam";
		mat<rgba> outimg;
		std::vector<srcimg> srcimgs;

		size_t n = 0;
		for (size_t i = 2; i < argc; i += n) {
			srcimg img;
			n = 0;
			if (std::string(argv[i]) == "-p") {
				n++;
				img.x_ = size_t(std::stoi(argv[i + n++]));
				img.y_ = size_t(std::stoi(argv[i + n++]));
			}
			img.name_ = std::string(argv[i + n++]) + ".pam";
			srcimgs.push_back(img);
		}

		for (const auto& si : srcimgs) {
			mat<rgba> img;
			pam::read_header(si.name_, img);
			if (outimg.cols() < img.cols() + si.x_) {
				outimg.resize(outimg.rows(), img.cols() + si.x_);
			}
			if (outimg.rows() < img.rows() + si.y_) {
				outimg.resize(img.rows() + si.y_, outimg.cols());
			}
		}
		for (const auto& si : srcimgs) {
			mat<rgba> img;
			pam::read(si.name_, img);
			for (size_t r = 0; r < img.rows(); r++) {
				for (size_t c = 0; c < img.cols(); c++) {
					rgba pixelA = img(r, c);
					rgba pixelB = outimg(r + si.y_, c + si.x_);
					rgba pixelO = compose_pixel(pixelA, pixelB);
					outimg(r + si.y_, c + si.x_) = pixelO;
				}
			}
		}

		pam::write(outimgname, outimg);
	//}
	//_CrtDumpMemoryLeaks();
	return EXIT_SUCCESS;
}