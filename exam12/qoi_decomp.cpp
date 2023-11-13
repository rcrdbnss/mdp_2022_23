#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

template<typename T>
T switch_endianness(T in, size_t n = sizeof(T)) {
    T ret = 0;
    for (int i = 0; i < n; i++) {
        ret |= ((in >> (8 * (n - 1 - i))) & 0xFF) << (8 * i);
    }
    return ret;
}

template<typename T>
struct mat {
    size_t rows_, cols_;
    std::vector<T> data_;

    mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows*cols) {}

    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }

    size_t size() const { return rows_ * cols_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    const char* rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t rawsize() const { return size() * sizeof(T); }
};

struct qoi_header{
    char magic[4];      // magic bytes "qoif"
    uint32_t width;     // image width in pixels (BE)
    uint32_t height;    // image height in pixels (BE)
    uint8_t channels;   // 3 = RGB, 4 = RGBA
    uint8_t colorspace; // 0 = sRGB with linear alpha, 1 = all channels linear
};

int main(int argc, char *argv[])
{
    // Manage the command line  
    std::string srcfname = argv[1];
    std::ifstream is(srcfname, std::ios::binary);
    if (!is) {
        return EXIT_FAILURE;
    }

    // Lettura dell'header e delle dimensioni dell'immagine 
    qoi_header h;
    is.read(reinterpret_cast<char*>(&h), 14);
    h.height = switch_endianness(h.height);
    h.width = switch_endianness(h.width);

    using rgba = std::array<uint8_t, 4>; // Potete modificare questa definizione!
    mat<rgba> img(h.height, h.width); 
    rgba pixel = { 0, 0, 0, 255 };
    if (h.channels == 4) pixel[3] = 0;  // alpha transparent
    std::vector<rgba> prevpx(64, pixel);

    // decodificare il file QOI in input e inserire i dati nell'immagine di output
    uint8_t byte = 0;
    uint8_t runs = 0;
    size_t len = 0;
    while (is.read(reinterpret_cast<char*>(&byte), 1)) {
        if (byte == 0xFF) {
            // rgba
            is.read(reinterpret_cast<char*>(&pixel), 4);
            runs = 1;
        }
        else if (byte == 0xFE) {
            // rgba
            is.read(reinterpret_cast<char*>(&pixel), 3);// keep alpha
            runs = 1;
        }
        else if (byte == 0x00) {
            // check if end marker or index
            size_t pos = is.tellg();
            uint8_t end_byte = 1;
            uint8_t end_bytes = 1;
            bool end = false;
            while (is.read(reinterpret_cast<char*>(&end_byte), 1) && end_byte == 0) end_bytes++;
            if (end_bytes == 7) {
                if (is.read(reinterpret_cast<char*>(&end_byte), 1) && end_byte == 1) end = true;
            }
            if (end) break;
            else {
                is.seekg(pos);
                uint8_t i = byte & 0x3F;    // 0
                pixel = prevpx[i];
                runs = 1;
            }
        }
        else if ((byte >> 6) == 0) {
            // index
            uint8_t i = byte & 0x3F;
            pixel = prevpx[i];
            runs = 1;
        }
        else if ((byte >> 6) == 1) {
            // 1 byte diff 
            int8_t dr = -2, dg = -2, db = -2;
            dr += (byte >> 4) & 0x03;
            dg += (byte >> 2) & 0x03;
            db += byte & 0x03;
            pixel[0] += dr;
            pixel[1] += dg;
            pixel[2] += db;
            runs = 1;

        }
        else if ((byte >> 6) == 2) {
            // 2 byte diff
            int8_t dg = -32, dr_dg = -8, db_dg = -8;
            dg += byte & 0x3F;
            pixel[1] += dg;

            byte = is.get();
            dr_dg += (byte >> 4) & 0x0F;
            db_dg += byte & 0x0F;
            pixel[0] += dr_dg + dg;
            pixel[2] += db_dg + dg;
            runs = 1;
        }
        else if ((byte >> 6) == 3) {
            // run
            runs = (byte & 0x3F) + 1;
        }
        prevpx[(pixel[0] * 3 + pixel[1] * 5 + pixel[2] * 7 + pixel[3] * 11) % 64] = pixel;
        for (int i = 0; i < runs; i++) {
            if (len + i < img.size()) {
                img.data_[len + i] = pixel;
            }
        }
        len += runs;
    }

    // Questo è il formato di output PAM. È già pronto così, ma potete modificarlo se volete
    std::ofstream os(argv[2], std::ios::binary); // Questo utilizza il secondo parametro della linea di comando!
    os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() <<
        "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.rawsize());

    return EXIT_SUCCESS;
}