#include "y4m_color.h"
#include <iostream>
#include <fstream>
#include "mat.h"
#include "types.h"

/*static*/ mat<double> get_YCbCr_to_RGB_coef() {
	mat<double> coef(3, 3);
	coef(0, 0) = 1.164;
	coef(0, 1) = 0.0;
	coef(0, 2) = 1.596;
	coef(1, 0) = 1.164;
	coef(1, 1) = -0.392;
	coef(1, 2) = -0.813;
	coef(2, 0) = 1.164;
	coef(2, 1) = 2.017;
	coef(2, 2) = 0.0;
	return coef;
}

/*static*/ mat<double> get_YCbCr_to_RGB_bias() {
	mat<double> bias(3, 1);
	bias(0, 0) = 16.0;
	bias(1, 0) = 128.0;
	bias(2, 0) = 128.0;
	return bias;
}

double upsample(const mat<uint8_t>& mat, int r, int c) {
	return mat(r / 2, c / 2);
}

template <typename T>
/*static*/ mat<T> matsum(const mat<T>& a, const mat<T>& b, mat<T>& sum) {
	int r = a.rows();
	int c = a.cols();
	sum.resize(r, c);
	// missing size check  
	for (int i = 0; i < r; i++) {
		for (int j = 0; j < c; j++) {
			sum(i, j) = a(i, j) + b(i, j);
		}
	}
	// return sum;
}

template <typename T>
/*static*/ void matdif(const mat<T>& a, const mat<T>& b, mat<T>& dif) {
	int r = a.rows();
	int c = a.cols();
	dif.resize(r, c);
	// missing size check  
	for (int i = 0; i < r; i++) {
		for (int j = 0; j < c; j++) {
			dif(i, j) = a(i, j) - b(i, j);
		}
	}
	// return dif;
}

template <typename T>
/*static*/ void matmul(const mat<T>& a, const mat<T>& b, mat<T>& mul) {
	int ra = a.rows();
	int cb = b.cols();
	int ca = a.cols();
	mul.resize(ra, cb);
	// missing size check
	for (int i = 0; i < ra; i++) {
		for (int j = 0; j < cb; j++) {
			mul(i, j) = 0;
			for (int k = 0; k < ca; k++) {
				mul(i, j) += a(i, k) * b(k, j);
			}
		}
	}
	// return mul;
}

struct StreamHeaderFields {
	std::string header;
	int height, width;
	std::string chroma_subsampling = "420jpeg";
	char interlacing = 'p';
	// other fields ignored
};

struct FrameHeaderFields {
	std::string header;
	std::string interlacing, appDepField;
};

bool readHeaderFields(std::ifstream& is, StreamHeaderFields& shf) {
	is >> shf.header;
	if (shf.header != "YUV4MPEG2") {
		std::cerr << "Error 2";
		return false;
	}

	char c;
	bool isWidth = false, isHeight = false;
	while (is.get(c) && c == ' ') {
		char tag = is.get();
		switch (tag) {
		case 'W':
			is >> shf.width;
			isWidth = true;
			break;
		case 'H':
			is >> shf.height;
			isHeight = true;
			break;
		case 'C':
			is >> shf.chroma_subsampling;
			break;
		case 'I':
			is >> shf.interlacing;
			break;
		default:
			std::string dump;
			is >> dump;
			break;
		}
	}
	return is && c == '\n' && isWidth && isHeight;
}

bool readFrameFields(std::ifstream& is, FrameHeaderFields& fhf) {
	is >> fhf.header;
	if (fhf.header != "FRAME") {
		return true;
	}

	char c;
	while (is.get(c) && c == ' ') {
		//char tag = is.get();
		is.get();
		std::string dump;
		is >> dump;
	}
	return is && c == '\n';
}

// @mem(&frame.data_[0], UINT8, 3, frame.cols_, frame.rows_, frame.cols_ * 3)
bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		std::cerr << "Failed to open the file " << filename;
		return false;
	}

	StreamHeaderFields hf;
	if (!readHeaderFields(is, hf)) {
		std::cerr << "Error 3";
		return false;
	}

	mat<vec3b> frame(hf.height, hf.width);
	mat<uint8_t> Y(hf.height, hf.width);
	mat<uint8_t> Cb(hf.height / 2, hf.width / 2);
	mat<uint8_t> Cr(hf.height / 2, hf.width / 2);

	const mat<double> bias = get_YCbCr_to_RGB_bias();
	const mat<double> coef = get_YCbCr_to_RGB_coef();

	while (true) {
		FrameHeaderFields ff;
		if (!readFrameFields(is, ff)) {
			std::cerr << "Error 4";
			return false;
		}
		// if this is not a FRAME, skip all the following and end the process successfully
		if (ff.header != "FRAME") {
			break;
			// return true;
		}

		is.read(Y.rawdata(), Y.rawsize());
		is.read(Cb.rawdata(), Cb.rawsize());
		is.read(Cr.rawdata(), Cr.rawsize());

		if (!is) {
			std::cerr << "Error 5";
			return false;
		}

		for (int r = 0; r < frame.rows(); r++) {
			for (int c = 0; c < frame.cols(); c++) {
				double y = Y(r, c);
				double cb = upsample(Cb, r, c);
				double cr = upsample(Cr, r, c);

				y = y < 16 ? 16 : y > 235 ? 235 : y;
				cb = cb < 16 ? 16 : cb > 240 ? 240 : cb;
				cr = cr < 16 ? 16 : cr > 240 ? 240 : cr;

				/*// fast version
				y -= 16;
				cb -= 128;
				cr -= 128;

				double dr = 1.164 * y + 0.000 * cb + 1.596 * cr;
				double dg = 1.164 * y - 0.392 * cb - 0.813 * cr;
				double db = 1.164 * y + 2.017 * cb + 0.000 * cr;

				dr = dr < 0 ? 0 : dr > 255 ? 255 : dr;
				dg = dg < 0 ? 0 : dg > 255 ? 255 : dg;
				db = db < 0 ? 0 : db > 255 ? 255 : db;

				frame(r, c) = { uint8_t(dr), uint8_t(dg), uint8_t(db) };
				// end fast*/
				
				// slow version
				mat<double> ycbcr(3, 1);
				ycbcr(0, 0) = y;
				ycbcr(1, 0) = cb;
				ycbcr(2, 0) = cr;

				mat<double> biased, rgb; 
				matdif(ycbcr, bias, biased);
				matmul(coef, biased, rgb);

				for (int k = 0; k < 3; k++) {
					double v = rgb(k, 0);
					v = v < 0 ? 0 : v > 255 ? 255 : v;
					rgb(k, 0) = v;
				}

				frame(r, c) = { 
					uint8_t(rgb(0, 0)), 
					uint8_t(rgb(1, 0)), 
					uint8_t(rgb(2, 0)) 
				};
				// end slow
			}
		}

		frames.push_back(frame);
	}
	return true;
}
