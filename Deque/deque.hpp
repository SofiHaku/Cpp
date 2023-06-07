/**
 * @file deque.hpp
 * @author SofiHaku
 */

#include <array>
#include <cstring>
#include <iostream>
#include <vector>

template <typename T, typename Alloc = std::allocator<T>>
class Deque {
 private:
  static const size_t kBucket = 10;
  static const size_t kMemory = 3;
  static const size_t kStartNumberBuckets = 2;
  size_t size_ = 0;
  size_t index_bucket_start_;
  size_t index_element_start_ = kBucket / 2;
  std::vector<T*> memory_;
  using alloc_traits = std::allocator_traits<Alloc>;
  Alloc alloc_;
  void get_real_index(size_t index,
                      std::pair<size_t, size_t>& real_index) const {
    real_index.first = index_bucket_start_ + index / kBucket;
    real_index.second = (index_element_start_ + index) % kBucket;
    if (index_element_start_ + index % kBucket >= kBucket) {
      ++real_index.first;
    }
  }
  void start_allocate(size_t count) {
    size_ = count;
    size_t number_used_buckets = size_ / kBucket + kStartNumberBuckets;
    index_bucket_start_ = number_used_buckets;
    memory_ = std::vector<T*>(number_used_buckets * kMemory, nullptr);
    for (size_t i = 0; i < number_used_buckets; ++i) {
      memory_[number_used_buckets + i] =
          alloc_traits::allocate(alloc_, kBucket);
    }
  }
  void add_allocate_back(std::pair<size_t, size_t>& real_index) {
    if (real_index.first >= memory_.size()) {
      std::vector<T*> new_memory(memory_.size() * kMemory, nullptr);
      for (size_t i = 0; i < memory_.size(); i++) {
        new_memory[memory_.size() + i] = memory_[i];
      }
      index_bucket_start_ += memory_.size();
      std::swap(new_memory, memory_);
      get_real_index(size_, real_index);
    }
    if (memory_[real_index.first] == nullptr) {
      memory_[real_index.first] = alloc_traits::allocate(alloc_, kBucket);
    }
  }
  void add_allocate_front() {
    if ((index_bucket_start_ == 0) && (index_element_start_ == 0)) {
      std::vector<T*> new_memory(memory_.size() * kMemory, nullptr);
      for (size_t i = 0; i < memory_.size(); i++) {
        new_memory[memory_.size() + i] = memory_[i];
      }
      index_bucket_start_ += memory_.size();
      std::swap(new_memory, memory_);
    }
    if ((index_element_start_ == 0) &&
        (memory_[index_bucket_start_ - 1] == nullptr)) {
      memory_[index_bucket_start_ - 1] =
          alloc_traits::allocate(alloc_, kBucket);
      index_bucket_start_--;
      index_element_start_ = kBucket;
    }
    index_element_start_--;
  }
 public:
  using allocator_type = Alloc;
  Deque(size_t count, const T& value, const Alloc& alloc = Alloc())
      : alloc_(alloc) {
    try {
      std::pair<size_t, size_t> real_index;
      start_allocate(count);
      for (size_t i = 0; i < size_; ++i) {
        try {
          get_real_index(i, real_index);
          alloc_traits::construct(
              alloc_, &memory_[real_index.first][real_index.second], value);
        } catch (...) {
          for (size_t j = 0; j < i; ++j) {
            get_real_index(j, real_index);
            alloc_traits::destroy(
                alloc_, &memory_[real_index.first][real_index.second]);
          }
          throw;
        }
      }
    } catch (...) {
      std::cout << "error in constructor" << std::endl;
      for (size_t i = 0; i < memory_.size(); i++) {
        alloc_traits::deallocate(alloc_, memory_[i], kBucket);
      }
      throw;
    }
  }
  Deque(const Alloc& alloc = Alloc()) : size_(0), alloc_(alloc) {}
  Deque(size_t count, const Alloc& alloc = Alloc()) : alloc_(alloc) {
    std::pair<size_t, size_t> real_index;
    try {
      start_allocate(count);
      for (size_t i = 0; i < size_; ++i) {
        try {
          get_real_index(i, real_index);
          alloc_traits::construct(
              alloc_, &memory_[real_index.first][real_index.second]);
        } catch (...) {
          for (size_t j = 0; j < i; ++j) {
            get_real_index(j, real_index);
            alloc_traits::destroy(
                alloc_, &memory_[real_index.first][real_index.second]);
          }
          throw;
        }
      }
    } catch (...) {
      std::cout << "error in constructor" << std::endl;
      for (size_t i = 0; i < memory_.size(); i++) {
        alloc_traits::deallocate(alloc_, memory_[i], kBucket);
      }
      throw;
    }
  }
  Deque(const Deque& other)
      : alloc_(
            alloc_traits::select_on_container_copy_construction(other.alloc_)) {
    std::pair<size_t, size_t> real_index;
    try {
      start_allocate(other.size());
      for (size_t i = 0; i < size_; ++i) {
        try {
          get_real_index(i, real_index);
          alloc_traits::construct(
              alloc_, &memory_[real_index.first][real_index.second], other[i]);
        } catch (...) {
          for (size_t j = 0; j < i; ++j) {
            get_real_index(j, real_index);
            alloc_traits::destroy(
                alloc_, &memory_[real_index.first][real_index.second]);
          }
          throw;
        }
      }
    } catch (...) {
      std::cout << "error in constructor" << std::endl;
      for (size_t i = 0; i < memory_.size(); i++) {
        alloc_traits::deallocate(alloc_, memory_[i], kBucket);
      }
      throw;
    }
  }
  Deque(Deque&& other)
      : size_(std::exchange(other.size_, 0)),
        index_bucket_start_(std::exchange(other.index_bucket_start_, 0)),
        index_element_start_(std::exchange(other.index_element_start_, 0)),
        memory_(std::move(other.memory_)) {
    if (alloc_traits::propagate_on_container_move_assignment::value) {
      alloc_ = std::move(other.alloc_);
    } else {
      alloc_ = Alloc();
    }
  }
  Deque(std::initializer_list<T> init, const Alloc& alloc = Alloc())
      : alloc_(alloc) {
    std::pair<size_t, size_t> real_index;
    auto elem = init.begin();
    start_allocate(init.size());
    for (size_t i = 0; i < size_; ++i, ++elem) {
      get_real_index(i, real_index);
      alloc_traits::construct(
          alloc_, &memory_[real_index.first][real_index.second], *elem);
    }
  }
  ~Deque() {
    std::pair<size_t, size_t> ind;
    for (size_t i = 0; i < size_; ++i) {
      get_real_index(i, ind);
      alloc_traits::destroy(alloc_, &memory_[ind.first][ind.second]);
    }
    for (size_t i = 0; i < memory_.size(); ++i) {
      alloc_traits::deallocate(alloc_, memory_[i], kBucket);
    }
  }
  Deque& operator=(const Deque& other) {
    try {
      Deque<T, Alloc> dop(other);
      std::swap(memory_, dop.memory_);
      std::swap(size_, dop.size_);
      std::swap(index_bucket_start_, dop.index_bucket_start_);
      std::swap(index_element_start_, dop.index_element_start_);
      if (alloc_traits::propagate_on_container_copy_assignment::value) {
        alloc_ = other.alloc_;
      }
      return *this;
    } catch (...) {
      std::cout << "error in operator=" << std::endl;
      throw;
    }
  }
  Deque& operator=(Deque&& other) {
    try {
      Deque<T, Alloc> dop = std::move(other);
      std::swap(memory_, dop.memory_);
      std::swap(size_, dop.size_);
      std::swap(index_bucket_start_, dop.index_bucket_start_);
      std::swap(index_element_start_, dop.index_element_start_);
      if (alloc_traits::propagate_on_container_move_assignment::value) {
        alloc_ = std::move(other.alloc_);
      }
      return *this;
    } catch (...) {
      std::cout << "error in operator=" << std::endl;
      throw;
    }
  }
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  T& operator[](size_t index) {
    std::pair<size_t, size_t> real_index;
    get_real_index(index, real_index);
    return memory_[real_index.first][real_index.second];
  }
  const T& operator[](size_t index) const {
    std::pair<size_t, size_t> real_index;
    get_real_index(index, real_index);
    return memory_[real_index.first][real_index.second];
  }
  T& at(size_t index) {
    if (index >= size_) {
      std::string index_str = std::to_string(index);
      std::string messages = "Out of range deque. Index = ";
      messages += index_str;
      throw std::out_of_range(messages.c_str());
    }
    return this->operator[](index);
  }
  const T& at(size_t index) const {
    if (index >= size_) {
      std::string index_str = std::to_string(index);
      std::string messages = "Out of range deque. Index = ";
      messages += index_str;
      throw std::out_of_range(messages.c_str());
    }
    return this->operator[](index);
  }
  void push_back(const T& new_value) {
    size_t size_before = size_;
    try {
      if (size_ == 0) {
        start_allocate(0);
      }
      std::pair<size_t, size_t> real_index;
      get_real_index(size_, real_index);
      add_allocate_back(real_index);
      alloc_traits::construct(
          alloc_, &memory_[real_index.first][real_index.second], new_value);
      size_++;
    } catch (...) {
      std::cout << "push_back_error" << std::endl;
      throw;
    }
  }
  void push_back(T&& new_value) {
    size_t size_before = size_;
    try {
      if (size_ == 0) {
        start_allocate(0);
      }
      std::pair<size_t, size_t> real_index;
      get_real_index(size_, real_index);
      add_allocate_back(real_index);
      alloc_traits::construct(alloc_,
                              &memory_[real_index.first][real_index.second],
                              std::move(new_value));
      size_++;
    } catch (...) {
      std::cout << "push_back_error" << std::endl;
      throw;
    }
  }
  template <typename... Args>
  void emplace_back(const Args&... args) {
    size_t size_before = size_;
    try {
      if (size_ == 0) {
        start_allocate(0);
      }
      std::pair<size_t, size_t> real_index;
      get_real_index(size_, real_index);
      add_allocate_back(real_index);
      alloc_traits::construct(
          alloc_, &memory_[real_index.first][real_index.second], args...);
      size_++;
    } catch (...) {
      std::cout << "push_back_error" << std::endl;
      throw;
    }
  }
  template <typename... Args>
  void emplace_back(Args&&... args) {
    size_t size_before = size_;
    try {
      if (size_ == 0) {
        start_allocate(0);
      }
      std::pair<size_t, size_t> real_index;
      get_real_index(size_, real_index);
      add_allocate_back(real_index);
      alloc_traits::construct(alloc_,
                              &memory_[real_index.first][real_index.second],
                              std::forward<Args>(args)...);
      size_++;
    } catch (...) {
      std::cout << "push_back_error" << std::endl;
      throw;
    }
  }
  void pop_back() {
    if (size_ >= 1) {
      size_--;
      std::pair<size_t, size_t> ind;
      get_real_index(size_, ind);
      alloc_traits::destroy(alloc_, &memory_[ind.first][ind.second]);
    }
  }
  void pop_front() {
    if (size_ < 1) {
      return;
    }
    std::pair<size_t, size_t> ind;
    get_real_index(0, ind);
    alloc_traits::destroy(alloc_, &memory_[ind.first][ind.second]);
    size_--;
    if (index_element_start_ == kBucket - 1) {
      index_element_start_ = 0;
      index_bucket_start_++;
    }
  }
  void push_front(const T& new_value) {
    try {
      if (size_ == 0) {
        start_allocate(0);
      }
      add_allocate_front();
      alloc_traits::construct(
          alloc_, &memory_[index_bucket_start_][index_element_start_],
          new_value);
      size_++;
    } catch (...) {
      std::cout << "push_front_error" << std::endl;
      throw;
    }
  }
  void push_front(T&& new_value) {
    try {
      if (size_ == 0) {
        start_allocate(0);
      }
      add_allocate_front();
      alloc_traits::construct(
          alloc_, &memory_[index_bucket_start_][index_element_start_],
          std::move(new_value));
      size_++;
    } catch (...) {
      std::cout << "push_back_error" << std::endl;
      throw;
    }
  }
  template <bool IsConst>
  struct CommonIterator {
   private:
    size_t first_index_;
    size_t second_index_;
    std::vector<T*>* memory_iterator_;
   public:
    using value_type = std::conditional_t<IsConst, const T, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    ~CommonIterator() {}
    reference operator*() {
      return (*memory_iterator_)[first_index_][second_index_];
    }
    pointer operator->() const {
      return &(*memory_iterator_)[first_index_][second_index_];
    }
    CommonIterator& operator=(CommonIterator other) {
      std::swap(first_index_, other.first_index_);
      std::swap(second_index_, other.second_index_);
      std::swap(memory_iterator_, other.memory_iterator_);
      return *this;
    }
    CommonIterator& operator++() {
      if (second_index_ == kBucket - 1) {
        second_index_ = 0;
        first_index_++;
      } else {
        second_index_++;
      }
      return *this;
    }
    CommonIterator operator++(int) {
      CommonIterator old = *this;
      operator++();
      return old;
    }
    CommonIterator& operator--() {
      if (second_index_ == 0) {
        second_index_ = kBucket - 1;
        first_index_--;
      } else {
        second_index_--;
      }
      return *this;
    }
    CommonIterator operator--(int) {
      CommonIterator old = *this;
      operator--();
      return old;
    }
    CommonIterator& operator+=(difference_type index) {
      if (second_index_ + index % kBucket >= kBucket) {
        first_index_ = first_index_ + index / kBucket + 1;
      } else {
        first_index_ = first_index_ + index / kBucket;
      }
      second_index_ = (second_index_ + index) % kBucket;
      return *this;
    }
    CommonIterator operator+(difference_type index) const {
      CommonIterator new_iter(*this);
      new_iter += static_cast<size_t>(index);
      return new_iter;
    }
    CommonIterator& operator-=(difference_type index) {
      if (static_cast<size_t>(index) <= second_index_) {
        second_index_ -= static_cast<size_t>(index);
        return *this;
      }
      int index_withput_second = index - second_index_;
      if (index_withput_second - kBucket * (index_withput_second / kBucket) >
          0) {
        first_index_ = first_index_ - index_withput_second / kBucket - 1;
      } else {
        first_index_ = first_index_ - index_withput_second / kBucket;
      }
      second_index_ = kBucket - (index_withput_second -
                                 kBucket * (index_withput_second / kBucket));
      return *this;
    }
    CommonIterator operator-(difference_type index) const {
      CommonIterator new_iter(*this);
      new_iter -= index;
      return new_iter;
    }
    difference_type operator-(const CommonIterator& other) {
      if (other.first_index_ == first_index_) {
        return second_index_ - other.second_index_;
      }
      return ((first_index_ - other.first_index_ - 1) * kBucket +
              (kBucket - other.second_index_) + second_index_);
    }
    CommonIterator(size_t index_first, size_t index_second,
                   std::vector<T*>& memory)
        : memory_iterator_(&memory),
          first_index_(index_first),
          second_index_(index_second) {}
    CommonIterator(const CommonIterator& other)
        : first_index_(other.first_index_),
          second_index_(other.second_index_),
          memory_iterator_(other.memory_iterator_) {}
    bool operator==(const CommonIterator& other) const {
      return (first_index_ == other.first_index_) &&
             (second_index_ == other.second_index_);
    }
    bool operator!=(const CommonIterator& other) const {
      return !(*this == other);
    }
    bool operator>(const CommonIterator& other) const {
      return (first_index_ > other.first_index_) ||
             ((first_index_ == other.first_index_) &&
              (second_index_ > other.second_index_));
    }
    bool operator<=(const CommonIterator& other) const {
      return !(*this > other);
    }
    bool operator<(const CommonIterator& other) const {
      return (*this <= other) && (*this != other);
    }
    bool operator>=(const CommonIterator& other) const {
      return !(*this < other);
    }
  };
  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  iterator begin() {
    return iterator(index_bucket_start_, index_element_start_, memory_);
  }
  iterator end() {
    std::pair<size_t, size_t> index;
    get_real_index(size_, index);
    return iterator(index.first, index.second, memory_);
  }
  const_iterator cbegin() {
    return const_iterator(index_bucket_start_, index_element_start_, memory_);
  }
  const_iterator cend() {
    std::pair<size_t, size_t> index;
    get_real_index(size_, index);
    return const_iterator(index.first, index.second, memory_);
  }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  void insert(iterator itr, const T& new_value) {
    if (size_ == 0 || itr == end()) {
      push_back(new_value);
      return;
    }
    push_back(*(end() - 1));
    for (auto it = end() - 1; it > itr; it--) {
      *it = *(it - 1);
    }
    *itr = new_value;
  }
  void erase(iterator itr) {
    for (auto it = itr; it != end() - 1; it++) {
      *it = *(it + 1);
    }
    pop_back();
  }
  Alloc& get_allocator() { return alloc_; }
};