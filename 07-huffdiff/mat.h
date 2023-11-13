#pragma once

template<typename T>
struct mat {
    size_t rows_;
    size_t cols_;
    std::vector<T> data_;

    mat(size_t rows = 0, size_t cols = 0) :
        rows_(rows), cols_(cols), data_(rows* cols) {}

    T& operator[] (size_t i) { return data_[i]; }
    const T& operator[] (size_t i) const { return data_[i]; }

    T& operator() (size_t r, size_t c) {
        return data_[r * cols_ + c];
    }
    const T& operator() (size_t r, size_t c) const {
        return data_[r * cols_ + c];
    }

    auto rows() const { return rows_; }
    auto cols() const { return cols_; }
    auto size() const { return rows_ * cols_; }

    auto rawdata() {
        return reinterpret_cast<char*>(&data_[0]);
    }
    auto rawdata() const {
        return reinterpret_cast<const char*>(&data_[0]);
    }
    auto rawsize() const { return size() * sizeof(T); }

    void resize(size_t rows, size_t cols) {
        rows_ = rows;
        cols_ = cols;
        data_.resize(rows * cols);
    }

    auto begin() { return data_.begin(); }
    auto begin() const { return data_.begin(); }
    auto end() { return data_.end(); }
    auto end() const { return data_.end(); }
};
