#include <fstream>
#include "pcx.h"

struct pcx_header {
	uint8_t		manufacturer;
	uint8_t		version;
	uint8_t		encoding;
	uint8_t		bits_per_plane;
	uint16_t	xmin;
	uint16_t	ymin;
	uint16_t	xmax;
	uint16_t	ymax;
	uint16_t	vert_dpi;
	uint16_t	horz_dpi;
	uint8_t		palette[48];
	uint8_t		reserved;
	uint8_t		color_planes;
	uint16_t	bytes_per_plane_line;
	uint16_t	palette_info;
	uint16_t	hor_scr_size;
	uint16_t	ver_scr_size;
	uint8_t		padding[54];
};

// Exercise 3
bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	pcx_header pcxh;
	is.read(reinterpret_cast<char*>(&pcxh), sizeof(pcx_header));

	vec3b palette[256];
	auto data_pos = is.tellg();
	is.seekg(-769, is.end);
	if (is.get() != 12) {
		return false;
	}
	is.read(reinterpret_cast<char*>(&palette), sizeof(palette));
	is.seekg(data_pos);

	uint16_t width = pcxh.xmax - pcxh.xmin + 1;
	uint16_t height = pcxh.ymax - pcxh.ymin + 1;
	uint64_t tot_bytes = uint64_t(pcxh.bytes_per_plane_line) * uint64_t(pcxh.color_planes);
	img.resize(height, width);

	for (int r = 0; r < height; r++) {
		for (uint64_t b = 0; b < tot_bytes; ) {
			uint8_t byte = 0;
			uint8_t to_read = 0;
			is.read(reinterpret_cast<char*>(&byte), 1);
			if ((byte & 0xC0) == 0xC0) {
				to_read = byte & 0x3F;
				is.read(reinterpret_cast<char*>(&byte), 1);
			}
			else {
				to_read = 1;
			}
			for (uint8_t i = 0; i < to_read; i++) {	// bytes
				if (b < tot_bytes) {
					img(r, b) = palette[byte];
				}
				b++;
			}
		}
	}

	return true;
}

// Exercise 2
bool load_pcx(const std::string& filename, mat<vec3b>& img, int ex2) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	pcx_header pcxh;
	is.read(reinterpret_cast<char*>(&pcxh), sizeof(pcx_header));

	uint16_t width = pcxh.xmax - pcxh.xmin + 1;
	uint16_t height = pcxh.ymax - pcxh.ymin + 1;
	uint64_t tot_bytes = uint64_t(pcxh.bytes_per_plane_line) * uint64_t(pcxh.color_planes);
	img.resize(height, width);

	for (int r = 0; r < height; r++) {
		for (uint64_t b = 0; b < tot_bytes; ) {
			uint8_t byte = 0;
			uint8_t to_read = 0;
			is.read(reinterpret_cast<char*>(&byte), 1);
			if ((byte & 0xC0) == 0xC0) {
				to_read = byte & 0x3F;
				is.read(reinterpret_cast<char*>(&byte), 1);
			}
			else {
				to_read = 1;
			}
			for (uint8_t i = 0; i < to_read; i++) {	// bytes
				if (b < tot_bytes && b % pcxh.bytes_per_plane_line < width) {
					int c = b % pcxh.bytes_per_plane_line;
					int v = b / pcxh.bytes_per_plane_line;
					img(r, c)[v] = byte;
 				}
				b++;
			}
		}
	}

	return true;
}

// Exercise 1
bool load_pcx(const std::string& filename, mat<uint8_t>& img) {
	std::ifstream is(filename, std::ios::binary);
	if (!is) {
		return false;
	}

	pcx_header pcxh;
	is.read(reinterpret_cast<char*>(&pcxh), sizeof(pcx_header));

	uint16_t width = pcxh.xmax - pcxh.xmin + 1;
	uint16_t height = pcxh.ymax - pcxh.ymin + 1;
	uint64_t tot_bytes = uint64_t(pcxh.bytes_per_plane_line) * uint64_t(pcxh.color_planes);
	img.resize(height, width);

	for (int r = 0; r < height; r++) {
		for (uint64_t b = 0; b < tot_bytes; ) {
			uint8_t byte = 0;
			uint8_t to_read = 0;
			is.read(reinterpret_cast<char*>(&byte), 1);
			if ((byte & 0xC0) == 0xC0) {
				to_read = byte & 0x3F;
				is.read(reinterpret_cast<char*>(&byte), 1);
			}
			else {
				to_read = 1;
			}
			for (uint8_t i = 0; i < to_read; i++) {	// bytes
				for (size_t j = 0; j < 8 && (b * 8 + j) < width; j++) {	// bits within the current byte
					img(r, b * 8 + j) = ((byte >> (7 - j)) & 1) * 255;
				}
				b++;
			}
		}
	}

	return true;
}
