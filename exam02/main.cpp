#include "base64.h"
#include <iostream>

int main(void) {
	std::cout << "ENCODE\n";
	std::cout << base64_encode("abc") << "\n";
	std::cout << base64_encode("1") << "\n";
	std::cout << base64_encode("12") << "\n";
	std::cout << base64_encode("short text") << "\n";
	std::cout << base64_encode("Pretty long text which requires more than 76 characters to encode it completely.") << "\n";
	std::cout << "\n";

	std::cout << "DECODE\n";
	std::cout << base64_decode("YWJj") << "\n";
	std::cout << base64_decode("MQ==") << "\n";
	std::cout << base64_decode("MTI=") << "\n";
	std::cout << base64_decode("MTJ=") << "\n";
	std::cout << base64_decode("AP8A") << "\n";
	std::cout << base64_decode("c2hvcnQgdGV4dA==") << "\n";
	std::cout << base64_decode("UHJldHR5IGxvbmcgdGV4dCB3aGljaCByZXF1aXJlcyBtb3JlIHRoYW4gNzYgY2hhcmFjdGVycyB0 \
		byBlbmNvZGUgaXQgY29tcGxldGVseS4=") << "\n";
	return EXIT_SUCCESS;
}