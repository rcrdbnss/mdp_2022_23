#include <array>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

// example1.txt.lz4 example1.dec.txt
// bibbia.txt.lz4 bibbia.dec.txt

int main(int argc, char** argv) {
	if (argc != 3) {
		return EXIT_FAILURE;
	}

	std::string srcfname = argv[1];
	std::string outfname = argv[2];

	std::ifstream is(srcfname, std::ios::binary);
	if (!is) return EXIT_FAILURE;

	std::ofstream os(outfname, std::ios::binary);
	if (!os) return EXIT_FAILURE;

	std::array<char, 4> lz4_magic = { 0x03, 0x21, 0x4C, 0x18 };
	std::array<char, 4> magic = { 0, 0, 0, 0 };
	is.read(reinterpret_cast<char*>(&magic), 4);
	if (magic != lz4_magic) return EXIT_FAILURE;

	uint32_t ulen = 0;	// uncompressed
	is.read(reinterpret_cast<char*>(&ulen), 4);

	char lz4const[4] = { 0, 0, 0, 0 };
	is.read(lz4const, 4);	// no check required in the assignment

	// while (is.read(reinterpret_cast<char*>(&blen), 4)) {
	while (true) {
		std::vector<char> buffer;
		uint32_t blen = 0;	// block
		is.read(reinterpret_cast<char*>(&blen), 4);
		if (!is) {
			break;
		}
		uint32_t bcount = 0; // block
		do {
			uint8_t token = 0;
			is.read(reinterpret_cast<char*>(&token), 1);
			bcount++;
			size_t litlen = (token >> 4) & 0x0F;
			if (litlen == 15) {
				uint8_t byte = 0;
				do {
					is.read(reinterpret_cast<char*>(&byte), 1);
					litlen += byte;
					bcount++;
				} while (byte == 0xFF);
			}

			//if ((bcount + litlen) == blen) {
			//	if (litlen < 5) {
			//		return EXIT_FAILURE; // last 5 bytes must be literals
			//	}
			//}

			if (litlen > 0) {
				std::string str = "";
				str.resize(litlen);
				// std::copy_n(std::istreambuf_iterator<char>(is), litlen, str.begin());
				is.read(reinterpret_cast<char*>(&str[0]), litlen);
				std::copy(str.begin(), str.end(), std::back_inserter(buffer));
				std::copy(str.begin(), str.end(), std::ostreambuf_iterator<char>(os));
				bcount += litlen;
				// is.get();
			}

			if (bcount < blen) {	// last sequence is literal, no match
				//uint16_t offset;
				uint16_t offset = 0;
				is.read(reinterpret_cast<char*>(&offset), 2);
				bcount += 2;

				size_t mtclen = token & 0x0F;
				if (mtclen == 15) {
					uint8_t byte = 0;
					do {
						is.read(reinterpret_cast<char*>(&byte), 1);
						mtclen += byte;
						bcount++;
					} while (byte == 0xFF);
				}
				mtclen += 4;
				size_t startpos = buffer.size() - offset;
				for (size_t i = 0; i < mtclen; i++) {
					char c = buffer[i + startpos];
					buffer.push_back(c);
					os.put(c);
				}
			}
		} while (bcount != blen);
	}
	//if (buffer.size() != ulen) {
	//	return EXIT_FAILURE;	// decompressed the wrong amount of data
	//}
	return EXIT_SUCCESS;
}