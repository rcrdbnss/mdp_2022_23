#pragma once
#include <vector>
#include <string>

struct BinaryImage {
    int W;
    int H;
    std::vector<uint8_t> ImageData;

    bool ReadFromPBM(const std::string& filename);
};

struct Image {
    int W;
    int H;
    std::vector<uint8_t> ImageData;
};

Image BinaryImageToImage(const BinaryImage& bimg);
