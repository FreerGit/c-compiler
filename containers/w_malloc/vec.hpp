#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <stdexcept>
#include <utility>

template <typename T> class Vector {
public:
  Vector() = default;

  Vector(std::size_t count) : sz_(0), capacity_(count) {
    data_ = static_cast<T *>(::operator new(sizeof(T) * capacity_));
  }

  ~Vector() {
    for (std::size_t i = 0; i < sz_; ++i)
      data_[i].~T();
    sz_ = 0;
    capacity_ = 0;
  }

  void push_back(const T &v) {
    printf("before s: %li c: %li", sz_, capacity_);
    if (sz_ == capacity_) {
      // realloc
      capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
      T *new_alloc = static_cast<T *>(::operator new(sizeof(T) * capacity_));
      if (!new_alloc)
        throw std::bad_alloc();

      for (size_t i = 0; i < sz_; ++i) {
        new (new_alloc + i) T(std::move(data_[i]));
        data_[i].~T();
      }

      free(data_);
      data_ = new_alloc;
    }

    new (data_ + sz_) T(v);
    ++sz_;
  }

  std::size_t size() const { return this->sz_; }

  bool empty() const { return this->sz_ == 0; }

  // element access
  const T &at(size_t idx) const {
    if (idx < 0 || idx >= sz_ || sz_ == 0)
      throw std::out_of_range("out of bounds");

    return data_[idx];
  }

  T &at(size_t idx) { return const_cast<T &>(std::as_const(*this).at(idx)); }

  const T &operator[](std::size_t at) const { return data_[at]; }
  T &operator[](std::size_t at) { return data_[at]; }

  const T &front() const { return this->at(0); }

  const T &back() const { return this->at(sz_ - 1); }

  const T *data() { return this->data_; }

private:
  T *data_ = nullptr;
  std::size_t sz_ = 0;
  std::size_t capacity_ = 0;
};
