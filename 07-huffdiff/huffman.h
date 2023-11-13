#pragma once
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <algorithm>

#include "bits.h"
#include "frequency.h"

template <typename T>
struct huffman {

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

	std::vector<code> codes_table_;

	template <typename M>
	void create_table(const M& map) {
		std::vector<std::unique_ptr<node>> storage;
		std::vector<node*> v;
		for (const auto& x : map) {
			node* n = new node(x.first, x.second);
			storage.emplace_back(n);
			v.push_back(n);
		}
		std::sort(begin(v), end(v), [](const node* a, const node* b) {
			return a->freq_ > b->freq_;
			});
		
		while (v.size() > 1) {
			// Take and remove the two least probable nodes
			node* n1 = v.back();
			v.pop_back();
			node* n2 = v.back();
			v.pop_back();
			// Combine the two in a new node
			node* n = new node(n1, n2);
			storage.emplace_back(n);
			// Add the new node into the vector in the correct position
			auto it = lower_bound(begin(v), end(v), n, 
				[](const node* a, const node* b) {
					return a->freq_ > b->freq_;
				});
			v.insert(it, n);
		}

		node* root = v.back();
		lengths(root, 0);
		sort(codes_table_.begin(), codes_table_.end());
	}

	void lengths(const node* p, uint32_t len) {
		if (p->left_ == nullptr) {
			code c = { p->sym_, len, 0 };
			codes_table_.push_back(c);
		}
		else {
			lengths(p->left_, len + 1);
			lengths(p->right_, len + 1);
		}
	}

	void canonical_codes() {
		code cur = { 0, 0, 0 };
		for (auto& x : codes_table_) {
			x.val_ = cur.val_ <<= x.len_ - cur.len_;
			cur.len_ = x.len_;
			++cur.val_;
		}
	}

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
		std::vector<uint8_t> v{
			std::istreambuf_iterator<char>(is),
			std::istreambuf_iterator<char>()
		};

		frequency_counter<T> f;
		f = std::for_each(v.begin(), v.end(), f);

		create_table(f);
		// canonical_codes();

		// Write output
		os << "HUFFMAN1";
		BitWriter bw(os);
		bw(f.size(), 8);
		for (const auto& x : codes_table_) {
			bw(x.sym_, 8);
			bw(x.len_, 5);
			bw(x.val_, x.len_);
		}
		bw(v.size(), 32);
		
		std::unordered_map<T, code> search_map;
		for (const auto& x : codes_table_) {
			search_map[x.sym_] = x;
		}
		for (const auto& x : v) {
			auto hc = search_map[x];
			bw(hc.val_, hc.len_);
		}
		return true;
	}
};
