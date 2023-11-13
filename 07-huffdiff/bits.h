#pragma once
#include <istream>

class BitReader {
private:
	uint8_t buffer_;
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

	std::istream& read(int32_t& i, size_t n) {
		uint32_t u;
		read(u, n);
		i = static_cast<int32_t>(u);
		return is_;
	}
};

class BitWriter {
	uint8_t buffer_;
	uint8_t nbits_ = 0;
	std::ostream& os_;

	std::ostream& write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | bit;
		++nbits_;
		// when the buffer is full, write it
		if (nbits_ == 8) {
			os_.write(reinterpret_cast<char*>(&buffer_), 1);
			nbits_ = 0;
		}
		return os_;
	}

public:
	BitWriter(std::ostream& os) : os_(os) {}

	~BitWriter() {
		flush();
	}

	std::ostream& write(uint32_t u, size_t n) {
		while (n-- > 0) {
			write_bit(u >> n);
		}
		return os_;
	}

	std::ostream& operator() (uint32_t u, size_t n) {
		return write(u, n);
	}

	void flush(uint8_t u = 0) {
		while (nbits_ > 0) {
			write_bit(u);
		}
	}

	template<typename T>
	static std::ostream& raw_write(std::ostream& os, const T& num, size_t size = sizeof(T)) {
		return os.write(reinterpret_cast<const char*>(&num), size);
	}
};
