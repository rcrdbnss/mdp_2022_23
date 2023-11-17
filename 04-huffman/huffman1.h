#pragma once
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <algorithm>

#include "bits.h"
#include "frequency.h"

template <typename T>
class huffman {
public:		// KEEP
	struct node {
		T sym_;
		uint32_t freq_;
		node* left_ = nullptr;
		node* right_ = nullptr;

		node(T sym, uint32_t freq) : sym_(sym), freq_(freq) {}

		node(node* left, node* right) :
			freq_(left->freq_ + right->freq_), left_(left), right_(right) {}

		~node() {
			delete left_;
			delete right_;
		}
	};

	struct code {
		T sym_;
		uint32_t len_, val_;
		bool operator< (const code& rhs) const {
			return len_ < rhs.len_;
		}
		bool operator> (const code& rhs) const {
			return len_ > rhs.len_;
		}
	};

private:	// KEEP
	std::vector<node*> nodes_tree_;
	std::vector<code> codes_table_;

	template <typename M>
	void createTree(const M& map) {
		// std::vector<std::unique_ptr<node>> storage;
		for (const auto& x : map) {
			node* n = new node(x.first, x.second);
			// storage.emplace_back(n);
			nodes_tree_.push_back(n);
		}
		std::sort(begin(nodes_tree_), end(nodes_tree_), [](const node* a, const node* b) {
			return a->freq_ > b->freq_;
			});

		while (nodes_tree_.size() > 1) {
			// Take and remove the two least probable nodes
			node* n1 = nodes_tree_.back();
			nodes_tree_.pop_back();
			node* n2 = nodes_tree_.back();
			nodes_tree_.pop_back();
			// Combine the two in a new node
			node* n = new node(n1, n2);
			// storage.emplace_back(n);
			// Add the new node into the vector in the correct position
			auto it = lower_bound(begin(nodes_tree_), end(nodes_tree_), n,
				[](const node* a, const node* b) {
					return a->freq_ > b->freq_;
				});
			nodes_tree_.insert(it, n);
		}
	}

private:	// ADAPT
	void generateCodesFromTree() {
		huffman::node* root = huffman::nodes_tree_.back();
		generateCodes(root);
	}

	void writeOutput(std::vector<uint8_t>& bytes, FrequencyCounter<T>& f, std::ofstream& os) {
		os << "HUFFMAN1";
		BitWriter bw(os);
		bw(f.size(), 8);
		for (const auto& x : huffman::codes_table_) {
			bw(x.sym_, 8);
			bw(x.len_, 5);
			bw(x.val_, x.len_);
		}
		bw(bytes.size(), 32);

		std::unordered_map<T, huffman::code> search_map;
		for (const auto& x : huffman::codes_table_) {
			search_map[x.sym_] = x;
		}
		for (const auto& x : bytes) {
			auto hc = search_map[x];
			bw(hc.val_, hc.len_);
		}
	}

	void readTable(BitReader& br) {
		uint32_t table_size;
		br.read(table_size, 8);
		for (uint32_t i = 0; i < table_size; i++) {
			code c;
			br.read(c.sym_, 8);
			br.read(c.len_, 5);
			br.read(c.val_, c.len_);
			codes_table_.push_back(c);
		}
		std::sort(codes_table_.begin(), codes_table_.end());
	}

private:	// ADAPT/DROP
	void generateCodes(const huffman<T>::node* p, uint32_t len = 0, uint32_t val = 0) {
		if (p->left_ == nullptr) {
			huffman::code c = { p->sym_, len, val };
			huffman::codes_table_.push_back(c);
		}
		else {
			generateCodes(p->left_, len + 1, val << 1 | 0);
			generateCodes(p->right_, len + 1, val << 1 | 1);
		}
	}

public:		// KEEP
	bool encode(const std::string& infile, const std::string& outfile) {
		// Open streams
		std::ifstream is(infile, std::ios::binary);
		if (!is) {
			return false;
		}
		std::ofstream os(outfile, std::ios::binary);
		if (!os) {
			return false;
		}

		// Load file
		std::vector<uint8_t> bytes{
			std::istreambuf_iterator<char>(is),
			std::istreambuf_iterator<char>()
		};

		FrequencyCounter<T> f;
		f = std::for_each(bytes.begin(), bytes.end(), f);

		createTree(f);
		generateCodesFromTree();
		writeOutput(bytes, f, os);
		return true;
	}

	bool decode(const std::string& infile, const std::string& outfile) {
		// Open streams
		std::ifstream is(infile, std::ios::binary);
		if (!is) {
			return false;
		}
		std::ofstream os(outfile, std::ios::binary);
		if (!os) {
			return false;
		}

		// Check header
		std::string header(8, ' ');
		BitReader::raw_read(is, header[0], 8);	// header[0] points to the actual data, 'string' is an object
		if (header != "HUFFMAN1") {
			return false; // error("header != \"HUFFMAN1\": wrong format");
		}

		// Read table
		BitReader br(is);
		readTable(br);

		uint32_t size;
		br.read(size, 32);
		for (uint32_t i = 0; i < size; i++) {
			uint32_t curlen = 0;
			uint32_t curval = 0;
			size_t j = 0;
			while (true) {
				auto cur_entry = codes_table_[j];
				while (curlen < cur_entry.len_) {
					curval = (curval << 1) | br.read_bit();
					curlen++;
				}
				if (curval == cur_entry.val_) {
					os.put(cur_entry.sym_);
					break;
				}
				j++;
				if (j >= codes_table_.size()) {
					return false;
				}
			}
		}
		return true;
	}
};
