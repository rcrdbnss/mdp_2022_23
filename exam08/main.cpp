#include "pcx.h"

int main(void) {
	/*mat<uint8_t> img;
	load_pcx("bunny.pcx", img);
	load_pcx("cat.pcx", img);
	load_pcx("clown.pcx", img);
	load_pcx("dog.pcx", img);
	load_pcx("hose.pcx", img);*/

	mat<vec3b> img;
	load_pcx("islanda_colori_8bit.pcx", img);
	return EXIT_SUCCESS;
}