#pragma once
#include "mat.h"
#include "types.h"
#include <string>

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames);
