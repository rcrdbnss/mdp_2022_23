#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "pam.h"

int main() {
    mat<uint8_t> img;
    if (!pam::read("frog.pam", img)) {
        return EXIT_FAILURE;
    }

    pam::vmirror(img);

    pam::write("output.pam", img);

    return 0;
}
