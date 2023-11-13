#include "process_ppm.h"
#include "compress.h"
#include "json.h"

int main(void) {
	std::string json = JSON("facolta.ppm");

	/*mat<vec3b> img;
	LoadPPM("test.ppm", img);
	mat<uint8_t> img_r; mat<uint8_t> img_g; mat<uint8_t> img_b;
	SplitRGB(img, img_r, img_g, img_b);
	std::vector<uint8_t> r, g, b;
	PackBitsEncode(img_r, r);
	PackBitsEncode(img_g, g);
	PackBitsEncode(img_b, b);
	std::string b64r, b64g, b64b;
	b64r = Base64Encode(r); 
	b64g = Base64Encode(g); 
	b64b = Base64Encode(b);

	mat<uint8_t> mt(5, 6);
	std::string str;

	str = "abcdefgggggggghhhhilmnooooopqr";
	for (int i = 0; i < str.size(); i++) {
		int r = i / 6;
		int c = i % 6;
		mt(r, c) = str[i];
	}
	std::vector<uint8_t> v;
	PackBitsEncode(mt, v);

	str = "abcdefgggggggghhhhilmnoooooooo";
	for (int i = 0; i < str.size(); i++) {
		int r = i / 6;
		int c = i % 6;
		mt(r, c) = str[i];
	}
	std::vector<uint8_t> u;
	PackBitsEncode(mt, u);*/

	return EXIT_SUCCESS;
}