#include "pbm.h"

int main(void) {
	std::string srcfname = "im2.pbm";
	BinaryImage bimg;
	bool res = bimg.ReadFromPBM(srcfname);
	if (res) {
		Image img = BinaryImageToImage(bimg);
	}
	return res ? EXIT_SUCCESS : EXIT_FAILURE;
}