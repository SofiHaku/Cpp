/**
 * @file list.hpp
 * @author SofiHaku
 */

#pragma once
#include <iostream>

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  struct Node {
    T value;
    Node* prev;
    Node* next;
    Node(const T& val) : value(val) {}
    Node() {}
    ~Node() {}
  };

  Node* head_ = nullptr;
  Node* tail_ = nullptr;
  size_t size_ = 0;

  using alloc_traits = std::allocator_traits<Alloc>;
  using node_alloc = typename alloc_traits::template rebind_alloc<Node>;
  using node_alloc_traits = typename alloc_traits::template rebind_traits<Node>;

  node_alloc alloc_;

 public:
  using value_type = T;
  using allocator_type = Alloc;
  List() : size_(0), alloc_(Alloc()) {}
  List(size_t count, const T& value, const Alloc& alloc = Alloc())
      : alloc_(alloc), size_(count) {
    size_ = count;
    head_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, head_, value);

    head_->prev = nullptr;
    Node* now_top = head_;

    for (size_t i = 0; i < count - 1; i++) {
      Node* dop = node_alloc_traits::allocate(alloc_, 1);
      node_alloc_traits::construct(alloc_, dop, value);
      now_top->next = dop;
      dop->prev = now_top;
      now_top = dop;
    }

    tail_ = node_alloc_traits::allocate(alloc_, 1);
    now_top->next = tail_;
    tail_->prev = now_top;
  }

  explicit List(size_t count, const Alloc& alloc = Alloc())
      : alloc_(alloc), size_(count) {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    tail_ = node_alloc_traits::allocate(alloc_, 1);
    node_alloc_traits::construct(alloc_, head_);
    Node* dop;
    Node* now_top = head_;

    for (size_t i = 0; i < count - 1; i++) {
      try {
        dop = node_alloc_traits::allocate(alloc_, 1);
        node_alloc_traits::construct(alloc_, dop);
        now_top->next = dop;
        dop->prev = now_top;
        now_top = dop;
      } catch (...) {
        node_alloc_traits::deallocate(alloc_, tail_, 1);
        node_alloc_traits::deallocate(alloc_, dop, 1);
        tail_ = now_top;
        while (head_ != tail_) {
          Node* old = tail_;
          tail_ = tail_->prev;
          node_alloc_traits::destroy(alloc_, old);
          node_alloc_traits::deallocate(alloc_, old, 1);
        }
        node_alloc_traits::destroy(alloc_, tail_);
        node_alloc_traits::deallocate(alloc_, tail_, 1);
        throw;
      }
    }

    now_top->next = tail_;
    tail_->prev = now_top;
  }

  List(std::initializer_list<T> init, const Alloc& alloc = Alloc())
      : alloc_(alloc), size_(init.size()) {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    Node* now_top = head_;
    std::cout << "second";

    for (auto& top : init) {
      try {
        node_alloc_traits::construct(alloc_, now_top, top);
        Node* dop = node_alloc_traits::allocate(alloc_, 1);
        now_top->next = dop;
        dop->prev = now_top;
        now_top = dop;
        tail_ = now_top;
      } catch (...) {
        while (head_ != tail_) {
          Node* old = head_;
          head_ = head_->next;
          node_alloc_traits::destroy(alloc_, old);
          node_alloc_traits::deallocate(alloc_, old, 1);
        }
        node_alloc_traits::deallocate(alloc_, head_, 1);
        throw;
      }
    }
  }

  List(const List& other)
      : alloc_(
            alloc_traits::select_on_container_copy_construction(other.alloc_)),
        size_(other.size()) {
    head_ = node_alloc_traits::allocate(alloc_, 1);
    Node* now_top = head_;

    for (auto it = other.begin(); it != other.end(); ++it) {
      try {
        node_alloc_traits::construct(alloc_, now_top, *it);
        Node* dop = node_alloc_traits::allocate(alloc_, 1);
        now_top->next = dop;
        dop->prev = now_top;
        now_top = dop;
        tail_ = now_top;
      } catch (...) {
        while (head_ != tail_) {
          Node* old = head_;
          head_ = head_->next;
          node_alloc_traits::destroy(alloc_, old);
          node_alloc_traits::deallocate(alloc_, old, 1);
        }
        node_alloc_traits::deallocate(alloc_, head_, 1);
        throw;
      }
    }
  }

  List<T, Alloc>& operator=(const List<T, Alloc>& other) {
    List<T, Alloc> dop(other);
    std::swap(head_, dop.head_);
    std::swap(tail_, dop.tail_);
    std::swap(size_, dop.size_);
    if (alloc_traits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc_;
    }
    return *this;
  }

  ~List() {
    while (head_ != tail_) {
      Node* old = head_;
      head_ = head_->next;
      node_alloc_traits::destroy(alloc_, old);
      node_alloc_traits::deallocate(alloc_, old, 1);
    }
    node_alloc_traits::deallocate(alloc_, head_, 1);
  }

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  node_alloc& get_allocator() { return alloc_; }

  void push_back(const T& value) {
    if (size_ == 0) {
      try {
        head_ = node_alloc_traits::allocate(alloc_, 1);
        tail_ = node_alloc_traits::allocate(alloc_, 1);
        node_alloc_traits::construct(alloc_, head_, value);
        head_->next = tail_;
        tail_->prev = head_;
      } catch (...) {
        node_alloc_traits::destroy(alloc_, head_);
        node_alloc_traits::deallocate(alloc_, head_, 1);
        node_alloc_traits::deallocate(alloc_, tail_, 1);
      }
    } else {
      Node* dop;
      try {
        dop = node_alloc_traits::allocate(alloc_, 1);
        node_alloc_traits::construct(alloc_, dop, value);
        dop->prev = tail_->prev;
        dop->next = tail_;
        tail_->prev->next = dop;
        tail_->prev = dop;
      } catch (...) {
        node_alloc_traits::destroy(alloc_, dop);
        node_alloc_traits::deallocate(alloc_, dop, 1);
      }
    }
    size_++;
  }

  void push_front(const T& value) {
    if (size_ == 0) {
      try {
        head_ = node_alloc_traits::allocate(alloc_, 1);
        tail_ = node_alloc_traits::allocate(alloc_, 1);
        node_alloc_traits::construct(alloc_, head_, value);
        head_->next = tail_;
        tail_->prev = head_;
      } catch (...) {
        node_alloc_traits::destroy(alloc_, head_);
        node_alloc_traits::deallocate(alloc_, head_, 1);
        node_alloc_traits::deallocate(alloc_, tail_, 1);
      }
    } else {
      Node* dop;
      try {
        dop = node_alloc_traits::allocate(alloc_, 1);
        node_alloc_traits::construct(alloc_, dop, value);
        dop->next = head_;
        head_ = dop;
      } catch (...) {
        node_alloc_traits::destroy(alloc_, dop);
        node_alloc_traits::deallocate(alloc_, dop, 1);
      }
    }
    size_++;
  }

  void pop_back() {
    Node* old_tail = tail_;
    tail_ = tail_->prev;
    node_alloc_traits::destroy(alloc_, old_tail);
    node_alloc_traits::deallocate(alloc_, old_tail, 1);
    size_--;
  }

  void pop_front() {
    Node* old_head = head_;
    head_ = head_->next;
    node_alloc_traits::destroy(alloc_, old_head);
    node_alloc_traits::deallocate(alloc_, old_head, 1);
    size_--;
  }

  template <bool IsConst>
  struct CommonIterator {
   private:
    Node* node_;

   public:
    using value_type = std::conditional_t<IsConst, const T, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

    ~CommonIterator() {}

    reference operator*() { return node_->value; }

    pointer operator->() const { return &(node_->value); }

    CommonIterator& operator=(CommonIterator other) {
      std::swap(node_, other.node_);
      return *this;
    }

    CommonIterator& operator++() {
      node_ = node_->next;
      return *this;
    }

    CommonIterator operator++(int) {
      CommonIterator old = *this;
      operator++();
      return old;
    }

    CommonIterator& operator--() {
      node_ = node_->prev;
      return *this;
    }

    CommonIterator operator--(int) {
      CommonIterator old = *this;
      operator--();
      return old;
    }

    CommonIterator(const CommonIterator& other) { node_ = other.node_; }

    CommonIterator(Node* node) { node_ = node; }

    bool operator==(const CommonIterator& other) const {
      return node_ == other.node_;
    }
    bool operator!=(const CommonIterator& other) const {
      return !(*this == other);
    }
  };

  using iterator = CommonIterator<false>;
  using const_iterator = CommonIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() const { return iterator(head_); }

  iterator end() const { return iterator(tail_); }

  const_iterator cbegin() { return const_iterator(true); }

  const_iterator cend() { return const_iterator(false); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
};