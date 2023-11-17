#pragma once

#include <unordered_map>

template<typename S, typename C = uint32_t>
class FrequencyCounter {
private:
	std::unordered_map<S, C> occurrencies_;

public:
	void operator()(const S& sym) {
		++occurrencies_[sym];
	}

	C& operator[](const S& sym) {
		return occurrencies_[sym];
	}

	auto begin() { return occurrencies_.begin(); }
	auto begin() const { return occurrencies_.begin(); }
	auto end() { return occurrencies_.end(); }
	auto end() const { return occurrencies_.end(); }

	double entropy() {
		double tot = 0.0;
		for (const auto& x : occurrencies_) {
			tot += x;
		}
		double H = 0.0;
		for (const auto& x : occurrencies_) {
			if (x > 0) {
				double px = x / tot;
				H += px * log2(px);
			}
		}
		return -H;
	}

	auto size() {
		return occurrencies_.size();
	}
};
