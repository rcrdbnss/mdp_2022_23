#define WINDOW_SIZE 2048

#include "lzs.h"
#include <vector>

/*template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}*/

class BitReader {
private:
	uint8_t buffer_ = 0;
	uint8_t nbits_ = 0;
	std::istream& is_;

public:
	BitReader(std::istream& is) : is_(is) {}

	int read_bit() {
		if (nbits_ == 0) {
			if (!raw_read(is_, buffer_)) {
				return EOF;
			}
			nbits_ = 8;
		}
		--nbits_;
		return (buffer_ >> nbits_) & 1;
	}

	explicit operator bool() { return bool(is_); }
	bool operator!() { return !is_; }

	template <typename T>
	std::istream& read(T& u, size_t n) {
		u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return is_;
	}

	template<typename T>
	static std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
		return is.read(reinterpret_cast<char*>(&val), size);
	}
};

uint16_t lzs_read_length(BitReader& reader) {
	uint16_t length = 0;
	uint8_t bits = 0;
	// reader.read_append(length, 2);
	reader.read(bits, 2);
	length += bits;
	if (length < 3) {
		return length + 2;
	}
	// reader.read_append(length, 2);
	reader.read(bits, 2);
	length = (length << 2) + (bits & 0x03);
	if (length < 15) {
		return length - 8 + 1;
	}
	uint16_t N = 0;
	do {
		N++;
		reader.read(bits, 4);
	} while (bits == 15);
	return bits + (N * 15 - 7);
}

void lzs_decompress(std::istream& is, std::ostream& os) {
	uint8_t byte = 0;
	std::vector<uint8_t> buffer_;

	BitReader reader(is);

	while (true) {
		if (reader.read_bit()) {
			// offset-length
			uint16_t offset = 0, length = 0;
			if (reader.read_bit()) {
				// offset < 128
				reader.read(offset, 7);
				if (offset == 0) {
					// end marker;
					break;
				}
			}
			else {
				// offset >= 128
				reader.read(offset, 11);
			}
			length = lzs_read_length(reader);

			size_t start_pos = buffer_.size() - offset;
			for (size_t i = 0; i < length; i++) {
				byte = buffer_[i + start_pos];
				buffer_.push_back(byte);
				os << byte;
			}
		}
		else {
			// literal
			reader.read(byte, 8);
			buffer_.push_back(byte);
			os << byte;
		}

		// free useless memory
		if (buffer_.size() > WINDOW_SIZE) {
			buffer_.erase(buffer_.begin(), buffer_.end() - WINDOW_SIZE);
		}
	}
	os.flush();
}
