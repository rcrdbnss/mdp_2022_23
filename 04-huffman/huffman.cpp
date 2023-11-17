#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "huffman1.h"

void syntax() {
	std::cout
		<< "SYNTAX:\n"
		<< "huffman [c|d] <input_file> <output_file>\n"
		<< "With the \"c\" option, the program opens the specified file (which must be treated as a binary file, that is, it can contain any value from 0 to 255 in each byte), calculates the frequencies and generates the corresponding Huffman codes.\n"
		<< "With the \"d\" optiom, the program decompresses the content of the input file (check that it is stored in the previous format) and saves it in the output file.\n"
		;
	exit(EXIT_FAILURE);
}

void error(const std::string& msg) {
	std::cout << "ERROR: " << msg << "\n";
}

//template <typename T>
//std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
//	return is.read(reinterpret_cast<char*>(&val), size);
//}
//
//template <typename T>
//std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
//	return os.write(reinterpret_cast<const char*>(&val), size);
//}

//class bitwriter {
//	uint8_t buffer_;
//	uint8_t nbits_ = 0;
//	std::ostream& os_;
//
//	void write_bit(uint8_t bit) {
//		buffer_ = (buffer_ << 1) | bit;
//		++nbits_;
//		// when the buffer is full, write it
//		if (nbits_ == 8) {
//			raw_write(os_, buffer_);
//			nbits_ = 0;
//		}
//	}
//
//public:
//	bitwriter(std::ostream& os) : os_(os) {}
//
//	~bitwriter() {
//		flush();
//	}
//
//	std::ostream& write(uint32_t u, uint8_t n) {
//		while (n-- > 0) {
//			uint8_t bit = (u >> n) & 1;
//			write_bit(bit);
//		}
//		return os_;
//	}
//
//	std::ostream& operator() (uint32_t u, uint8_t n) {
//		return write(u, n);
//	}
//
//	void flush(uint8_t bit = 0) {
//		while (nbits_ > 0) {
//			write_bit(bit);
//		}
//	}
//};
//
//struct bitreader {
//	uint8_t buffer_;
//	uint8_t nbits_ = 0;
//	std::istream& is_;
//
//	bitreader(std::istream& is) : is_(is) {}
//
//	int read_bit() {
//		if (nbits_ == 0) {
//			if (!raw_read(is_, buffer_)) {
//				return EOF;
//			}
//			nbits_ = 8;
//		}
//		--nbits_;
//		return (buffer_ >> nbits_) & 1;
//	}
//
//	explicit operator bool() { return bool(is_); }
//	bool operator!() { return !is_; }
//
//	std::istream& read(uint32_t& u, uint8_t n) {
//		u = 0;
//		while (n-- > 0) {
//			u = (u << 1) | read_bit();
//		}
//		return is_;
//	}
//};

struct huff_node {
	uint8_t sym_;
	uint32_t freq_;

	uint32_t len_;
	uint32_t code_;

	huff_node* left_ = nullptr;
	huff_node* right_ = nullptr;

	huff_node(uint8_t sym, uint32_t freq) : sym_(sym), freq_(freq) {}

	huff_node(huff_node* left, huff_node* right) :
		freq_(left->freq_ + right->freq_), left_(left), right_(right) {}

	~huff_node() {
		delete left_;
		delete right_;
	}

	void generate_codes(std::unordered_map<uint8_t, huff_node*>& table,
		uint32_t len = 0, uint32_t code = 0) {
		if (left_ == nullptr /* && right_ == nullptr */) {
			len_ = len;
			code_ = code;
			table[sym_] = this;
		}
		else {
			left_->generate_codes(table, len + 1, code * 2 + 0); // (code << 1) | 0
			right_->generate_codes(table, len + 1, code * 2 + 1); // (code << 1) | 1
		}
	}
};

void compress(const std::string& infile, const std::string& outfile) {
	// Open streams
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		error("Cannot open input file");
	}
	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		error("Cannot open output file");
	}

	// Load file
	std::vector<uint8_t> nodes_tree_{
		std::istreambuf_iterator<char>(is),
		std::istreambuf_iterator<char>()
	};

	// Compute frequencies
	std::unordered_map<uint8_t, uint32_t> count;
	for (const auto& x : nodes_tree_) {
		++count[x];
	}

	// Compute Huffman codes
	std::vector<huff_node*> huff_list;
	for (const auto& x : count) {
		huff_list.push_back(new huff_node(x.first, x.second));
	}
	sort(huff_list.begin(), huff_list.end(),
		[](const huff_node* lhs, const huff_node* rhs) {
			if (lhs->freq_ == rhs->freq_) {
				return lhs->sym_ < rhs->sym_;
			}
			return lhs->freq_ > rhs->freq_;
		});
	while (huff_list.size() > 1) {
		auto a = huff_list.back();
		huff_list.pop_back();
		auto b = huff_list.back();
		huff_list.pop_back();
		auto c = new huff_node(a, b);
		auto it = std::lower_bound(huff_list.begin(), huff_list.end(), c,
			[](const huff_node* lhs, const huff_node* rhs) {
				return lhs->freq_ > rhs->freq_;
			});
		huff_list.insert(it, c);
	}

	// Generate Huffman codes
	auto huff_root = huff_list.back();
	huff_list.pop_back();
	std::unordered_map<uint8_t, huff_node*> huff_table;
	huff_root->generate_codes(huff_table);

	// Write output
	os << "HUFFMAN1";
	BitWriter bw(os);
	bw(huff_table.size(), 8);
	for (const auto& x : huff_table) {
		bw(x.second->sym_, 8);
		bw(x.second->len_, 5);
		bw(x.second->code_, x.second->len_);
	}
	bw(nodes_tree_.size(), 32);
	for (const auto& x : nodes_tree_) {
		auto n = huff_table[x];
		bw(n->code_, n->len_);
	}

	// Free memory
	delete huff_root;
}

void decompress(const std::string& infile, const std::string& outfile) {
	// Open streams
	std::ifstream is(infile, std::ios::binary);
	if (!is) {
		error("Cannot open input file");
	}
	std::ofstream os(outfile, std::ios::binary);
	if (!os) {
		error("Cannot open output file");
	}

	// Check header
	std::string header(8, ' ');
	BitReader::raw_read(is, header[0], 8);	// header[0] points to the actual data, 'string' is an object
	if (header != "HUFFMAN1") {
		error("header != \"HUFFMAN1\": wrong format");
	}

	// Read table
	BitReader br(is);
	uint32_t table_size;
	br.read(table_size, 8);
	struct triplet {
		uint32_t sym;
		uint32_t len;
		uint32_t code;
	};
	std::vector<triplet> huff_table;
	for (uint32_t i = 0; i < table_size; i++) {
		triplet t;
		br.read(t.sym, 8);
		br.read(t.len, 5);
		br.read(t.code, t.len);
		huff_table.push_back(t);
	}
	sort(huff_table.begin(), huff_table.end(),
		[](const triplet& lhs, const triplet &rhs) {
			return lhs.len < rhs.len;
		});
	uint32_t size;
	br.read(size, 32);
	for (uint32_t i = 0; i < size; i++) {
		uint32_t curlen = 0;
		uint32_t curcode = 0;
		size_t pos = 0;
		while (true) {
			auto cur_entry = huff_table[pos];
			while (curlen < cur_entry.len) {
				curcode = (curcode << 1) | br.read_bit();
				++curlen;
			}
			if (curcode == cur_entry.code) {
				os.put(cur_entry.sym);
				break;
			}
			++pos;
			if (pos >= huff_table.size()) {
				error("Huffman code not found");
			}
		}
	}
}

int main(int argc, char* argv[])
{
	 {
		if (argc != 4) {
			syntax();
		}
		std::string command = argv[1];
		std::string input_filename = argv[2];
		std::string output_filename = argv[3];

		huffman<uint8_t> h;

		if (command == "c") {
			h.encode(input_filename, output_filename);
		}
		else if (command == "d") {
			h.decode(input_filename, output_filename);
			// decompress(input_filename, output_filename);
		}
		else {
			error("Command must be c or d");
		}
	 }
	 _CrtDumpMemoryLeaks();
	return EXIT_SUCCESS;
}

