//#include "y4m_gray.h"
#include "mat.h"
#include <fstream>
#include <iostream>

namespace y4m_gray {

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

	bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
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

		mat<uint8_t> Y(hf.height, hf.width);
		mat<uint8_t> Cb(hf.height / 2, hf.width / 2);
		mat<uint8_t> Cr(hf.height / 2, hf.width / 2);

		while (true) {
			FrameHeaderFields ff;
			if (!readFrameFields(is, ff)) {
				std::cerr << "Error 4";
				return false;
			}
			// if this is not a FRAME, skip all the following and end the process successfully
			if (ff.header != "FRAME") {
				return true;
			}

			is.read(Y.rawdata(), Y.rawsize());
			is.read(Cb.rawdata(), Cb.rawsize());
			is.read(Cr.rawdata(), Cr.rawsize());

			if (!is) {
				std::cerr << "Error 5";
				return false;
			}
			frames.push_back(Y);
		}
		return true;
	}

}