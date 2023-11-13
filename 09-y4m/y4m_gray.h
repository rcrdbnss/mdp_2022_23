#pragma once

#include "mat.h"
#include <string>

namespace y4m_gray {
	
	bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames);

}
