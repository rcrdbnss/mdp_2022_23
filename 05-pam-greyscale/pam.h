#pragma once
#include <fstream>
#include <iostream>
#include <string>

#include "matrix.h"

class pam {
public:
    
    static bool write(const std::string& filename, const mat<uint8_t>& img) {
        std::ofstream os(filename, std::ios::binary);
        if (!os) {
            return false;
        }

        os << "P7\n";
        os << "WIDTH " << img.cols() << "\n";
        os << "HEIGHT " << img.rows() << "\n";
        os << "DEPTH 1\n";
        os << "MAXVAL 255\n";
        os << "TUPLTYPE GRAYSCALE\n";
        os << "ENDHDR\n";

        os.write(img.rawdata(), img.rawsize());
        return true;
    }

    static bool read(const std::string& filename, mat<uint8_t>& img) {
        std::ifstream is(filename, std::ios::binary);
        if (!is) {
            return false;
        }

        std::string magic_number;
        is >> magic_number;
        if (magic_number != "P7") {
            return false;
        }

        bool has_width, has_height, has_depth, has_maxval, has_tupltype;
        has_width = has_height = has_depth = has_maxval = has_tupltype = false;

        std::string token;
        size_t width, height, depth, maxval;
        std::string tupltype;
        bool loop = true;
        while (is >> token) {
            if (token == "WIDTH") {
                is >> width;
                has_width = true;
            }
            else if (token == "HEIGHT") {
                is >> height;
                has_height = true;
            }
            else if (token == "DEPTH") {
                is >> depth;
                has_depth = true;
            }
            else if (token == "MAXVAL") {
                is >> maxval;
                has_maxval = true;
            }
            else if (token == "TUPLTYPE") {
                is >> tupltype;
                has_tupltype = true;
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

        if (!has_width || !has_height || !has_depth ||
            !has_maxval || !has_tupltype || 
            depth != 1 || maxval != 255 || tupltype != "GRAYSCALE") {
            return false;
        }

        img.resize(height, width);
        return is.read(img.rawdata(), img.rawsize()).good();
    }

    static void hmirror(mat<uint8_t>& img) {
        for (size_t r = 0; r < img.rows() / 2; r++) {
            for (size_t c = 0; c < img.cols(); c++) {
                std::swap(img(r, c), img(img.rows() - 1 - r, c));
            }
        }
    }

    static void vmirror(mat<uint8_t>& img) {
        for (size_t r = 0; r < img.rows(); r++) {
            for (size_t c = 0; c < img.cols() / 2; c++) {
                std::swap(img(r, c), img(r, img.cols() - 1 - c));
            }
        }
    }

    static mat<uint8_t>& basicimg(size_t rows = 256, size_t cols = 256) {
        mat<uint8_t> img(rows, cols);

        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < cols; c++) {
                img(r, c) = uint8_t(r);
            }
        }

        return img;
    }

};
