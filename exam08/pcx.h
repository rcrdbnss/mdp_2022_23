#pragma once
#include <string>
#include "mat.h"
#include "types.h"

bool load_pcx(const std::string& filename, mat<uint8_t>& img);

bool load_pcx(const std::string& filename, mat<vec3b>& img);
