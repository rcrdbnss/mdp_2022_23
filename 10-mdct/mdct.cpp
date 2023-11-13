#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>

template<typename CT, typename ST>
class MDCT {
private:
	std::vector<double> sin_tab_;
	std::vector<std::vector<double>> cos_tab_;
	const size_t N_;

	// template<typename CT, typename ST>
	void winTransform(const std::vector<ST>& padded, std::vector<CT>& coeffs, size_t winidx) {
		for (size_t k = 0; k < N_; k++) {
			double Xk = 0.;
			for (size_t n = 0; n < 2 * N_; n++) {
				ST xn = padded[winidx * N_ + n];
				double wn = sin_tab_[n];
				double cn = cos_tab_[k][n];

				Xk += xn * cn * wn;
			}
			coeffs[winidx * N_ + k] = CT(round(Xk));
		}
	}

	std::vector<double> winInvert(const std::vector<CT>& coeffs, size_t winidx) {
		using namespace std;
		vector<double> Y(2 * N_);

		for (size_t n = 0; n < 2 * N_; n++) {
			double tmp = 0.;
			for (size_t k = 0; k < N_; k++) {
				CT xk = coeffs[winidx * N_ + k];
				double ck = cos_tab_[k][n];
				tmp += ck * xk;
			}

			Y[n] = 2. / N_ * sin_tab_[n] * tmp;
		}

		return Y;
	}

public:
	MDCT(size_t N) : N_(N) {
		sin_tab_.resize(2 * N_);
		for (size_t i = 0; i < sin_tab_.size(); ++i)
			sin_tab_[i] = sin((M_PI / (2 * N_)) * (i + 0.5));

		cos_tab_ = std::vector<std::vector<double>>(N_, std::vector<double>(2 * N_));
		for (size_t k = 0; k < N_; k++) {
			for (size_t n = 0; n < 2 * N_; n++) {
				cos_tab_[k][n] = cos(M_PI / N_ * (n + 0.5 + N_ / 2) * (k + 0.5));
			}
		}
	}

	// template<typename CT, typename ST>
	std::vector<CT> transform(const std::vector<ST>& samples) {
		using namespace std;
		auto nwin = size_t(ceil(samples.size() / double(N_))) + 2;
		vector<ST> padded(nwin * N_, 0);
		// skip first block of N_ samples, left to 0
		copy(begin(samples), end(samples), begin(padded) + N_);

		vector<CT> coeffs((nwin - 1) * N_);
		for (size_t w = 0; w < nwin - 1; w++) {
			winTransform(padded, coeffs, w);
		}
		return coeffs;
	}

	std::vector<ST> invert(const std::vector<CT>& coeffs) {
		using namespace std;
		auto nwin = coeffs.size() / N_;
		vector<ST> samples((nwin - 1) * N_);

		vector<double> prev = winInvert(coeffs, 0);
		for (size_t i = 1; i < nwin; ++i) {
			vector<double> curr = winInvert(coeffs, i);

			for (size_t j = 0; j < N_; j++)
				samples[(i - 1) * N_ + j] = ST(round(curr[j] + prev[N_ + j]));

			prev = move(curr);
		}

		return samples;
	}
};

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
};

template <typename T>
void printFrequencyEntropy(const std::vector<T>& samples) {
	using namespace std;
	frequency_counter<T> f;
	f = for_each(begin(samples), end(samples), f);
	cout << "Number of different values: " << f.size() << "\n";
	cout << "Entropy: " << f.entropy() << "\n";
}

template <typename T>
std::vector<T> readTrack(const std::string& fname) {
	using namespace std;
	ifstream is(fname, ios::binary);
	is.seekg(0, ios::end);
	auto size = is.tellg();
	is.seekg(0, ios::beg);

	vector<T> samples(size_t(size) / sizeof(T));
	is.read(reinterpret_cast<char*>(samples.data()), size);
	return samples;
}

template <typename T>
void writeTrack(const std::string& fname, const std::vector<T>& samples) {
	using namespace std;
	ofstream os(fname, std::ios::binary);
	os.write(reinterpret_cast<const char*>(samples.data()),
		samples.size() * sizeof(T));
}

template <typename T>
T quantize(const T& val, double factor) {
	return T(lround(val / factor));
}

template <typename T>
T dequantize(const T& val, double factor) {
	return T(lround(val * factor));
}

int main(void) {
	using namespace std::placeholders;
	using datatype = int16_t;
	using coeftype = int32_t;

	auto original = readTrack<datatype>("test.raw");

	std::cout << "Info su campioni:\n";
	printFrequencyEntropy(original);

	// quantize in time
	if (true) {
		double nquant = 2600;
		auto fquant = bind(quantize<datatype>, _1, nquant);
		auto frecon = bind(dequantize<datatype>, _1, nquant);

		// quantized
		std::vector<datatype> quant_time(original.size());
		transform(begin(original), end(original), begin(quant_time), fquant);

		std::cout << "Info su campioni quantizzati:\n";
		printFrequencyEntropy(quant_time);

		// dequantize
		std::vector<datatype> recon(original.size());
		transform(begin(quant_time), end(quant_time), begin(recon), frecon);

		writeTrack("output_qt.raw", recon);

		// compute error
		std::vector<datatype> error(original.size());
		for (size_t i = 0; i < original.size(); i++)
			error[i] = original[i] - recon[i];

		writeTrack("error_qt.raw", error);
	}

	// quantize in frequency
	if (true) {
		double nquant = 20000;
		auto fquant = bind(quantize<coeftype>, _1, nquant);
		auto frecon = bind(dequantize<coeftype>, _1, nquant);
		// N is the number of coefficients
		size_t N = 1024;

		MDCT<coeftype, datatype> mdct(N);
		auto coef = mdct.transform(original);

		std::cout << "Info su coefficienti:\n";
		printFrequencyEntropy(coef);

		// quantize
		std::vector<coeftype> quant_coef(coef.size());
		transform(begin(coef), end(coef), begin(quant_coef), fquant);

		std::cout << "Info su coefficienti quantizzati:\n";
		printFrequencyEntropy(quant_coef);

		// dequantize
		std::vector<coeftype> recon_coef(coef.size());
		transform(begin(quant_coef), end(quant_coef), begin(recon_coef), frecon);

		auto recon = mdct.invert(coef);

		writeTrack("output.raw", recon);

		std::vector<datatype> error(recon.size());
		for (size_t i = 0; i < original.size(); i++) {
			error[i] = original[i] - recon[i];
		}

		writeTrack("error.raw", error);
	}
	return 0;
}
