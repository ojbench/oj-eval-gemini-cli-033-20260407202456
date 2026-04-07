#ifndef SJTU_LIST_HPP
#define SJTU_LIST_HPP

#include <cstddef>
#include <stdexcept>
#include <memory>

namespace sjtu {

template <typename T> class list {
protected:
  struct node_base {
    node_base *prev, *next;
    node_base() : prev(this), next(this) {}
  };
  struct node : public node_base {
    alignas(T) char data[sizeof(T)];
    node(const T &v) : node_base() {
      new (data) T(v);
    }
    ~node() {
      reinterpret_cast<T*>(data)->~T();
    }
    T* valptr() { return reinterpret_cast<T*>(data); }
    const T* valptr() const { return reinterpret_cast<const T*>(data); }
  };

  node_base *head;
  size_t sz;

  node_base *insert_node(node_base *pos, node_base *cur) {
    cur->next = pos;
    cur->prev = pos->prev;
    pos->prev->next = cur;
    pos->prev = cur;
    sz++;
    return cur;
  }

  node_base *erase_node(node_base *pos) {
    pos->prev->next = pos->next;
    pos->next->prev = pos->prev;
    sz--;
    return pos;
  }

public:
  class const_iterator;
  class iterator {
    friend class list<T>;
    friend class const_iterator;
  private:
    node_base *ptr;
    const list<T> *lst;
  public:
    iterator(node_base *p = nullptr, const list<T> *l = nullptr) : ptr(p), lst(l) {}
    
    iterator operator++(int) {
      iterator tmp = *this;
      if (!ptr || ptr == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->next;
      return tmp;
    }
    iterator &operator++() {
      if (!ptr || ptr == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->next;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp = *this;
      if (!ptr || ptr->prev == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->prev;
      return tmp;
    }
    iterator &operator--() {
      if (!ptr || ptr->prev == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->prev;
      return *this;
    }

    T &operator*() const {
      if (!ptr || ptr == lst->head) throw std::runtime_error("invalid");
      return *(static_cast<node*>(ptr)->valptr());
    }
    T *operator->() const noexcept {
      if (!ptr || ptr == lst->head) return nullptr;
      return static_cast<node*>(ptr)->valptr();
    }

    bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
    bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
    bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
    bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
  };

  class const_iterator {
    friend class list<T>;
    friend class iterator;
  private:
    node_base *ptr;
    const list<T> *lst;
  public:
    const_iterator(node_base *p = nullptr, const list<T> *l = nullptr) : ptr(p), lst(l) {}
    const_iterator(const iterator &other) : ptr(other.ptr), lst(other.lst) {}
    
    const_iterator operator++(int) {
      const_iterator tmp = *this;
      if (!ptr || ptr == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->next;
      return tmp;
    }
    const_iterator &operator++() {
      if (!ptr || ptr == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->next;
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator tmp = *this;
      if (!ptr || ptr->prev == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->prev;
      return tmp;
    }
    const_iterator &operator--() {
      if (!ptr || ptr->prev == lst->head) throw std::runtime_error("invalid");
      ptr = ptr->prev;
      return *this;
    }

    const T &operator*() const {
      if (!ptr || ptr == lst->head) throw std::runtime_error("invalid");
      return *(static_cast<node*>(ptr)->valptr());
    }
    const T *operator->() const noexcept {
      if (!ptr || ptr == lst->head) return nullptr;
      return static_cast<node*>(ptr)->valptr();
    }

    bool operator==(const iterator &rhs) const { return ptr == rhs.ptr; }
    bool operator==(const const_iterator &rhs) const { return ptr == rhs.ptr; }
    bool operator!=(const iterator &rhs) const { return ptr != rhs.ptr; }
    bool operator!=(const const_iterator &rhs) const { return ptr != rhs.ptr; }
  };

  list() {
    head = new node_base();
    sz = 0;
  }
  list(const list &other) {
    head = new node_base();
    sz = 0;
    try {
      for (auto it = other.cbegin(); it != other.cend(); ++it) {
        push_back(*it);
      }
    } catch (...) {
      clear();
      delete head;
      throw;
    }
  }
  virtual ~list() {
    clear();
    delete head;
  }
  list &operator=(const list &other) {
    if (this == &other) return *this;
    list tmp(other);
    node_base *tmp_head = head;
    head = tmp.head;
    tmp.head = tmp_head;
    size_t tmp_sz = sz;
    sz = tmp.sz;
    tmp.sz = tmp_sz;
    return *this;
  }

  const T &front() const {
    if (empty()) throw std::runtime_error("empty");
    return *(static_cast<node*>(head->next)->valptr());
  }
  const T &back() const {
    if (empty()) throw std::runtime_error("empty");
    return *(static_cast<node*>(head->prev)->valptr());
  }
  T &front() {
    if (empty()) throw std::runtime_error("empty");
    return *(static_cast<node*>(head->next)->valptr());
  }
  T &back() {
    if (empty()) throw std::runtime_error("empty");
    return *(static_cast<node*>(head->prev)->valptr());
  }

  iterator begin() { return iterator(head->next, this); }
  const_iterator begin() const { return const_iterator(head->next, this); }
  const_iterator cbegin() const { return const_iterator(head->next, this); }
  iterator end() { return iterator(head, this); }
  const_iterator end() const { return const_iterator(head, this); }
  const_iterator cend() const { return const_iterator(head, this); }

  virtual bool empty() const { return sz == 0; }
  virtual size_t size() const { return sz; }

  virtual void clear() {
    while (!empty()) {
      pop_back();
    }
  }

  virtual iterator insert(iterator pos, const T &value) {
    if (pos.lst != this) throw std::runtime_error("invalid");
    node *n = new node(value);
    insert_node(pos.ptr, n);
    return iterator(n, this);
  }

  virtual iterator erase(iterator pos) {
    if (pos.lst != this || pos.ptr == head) throw std::runtime_error("invalid");
    node_base *nxt = pos.ptr->next;
    erase_node(pos.ptr);
    delete static_cast<node*>(pos.ptr);
    return iterator(nxt, this);
  }

  void push_back(const T &value) {
    node *n = new node(value);
    insert_node(head, n);
  }
  void pop_back() {
    if (empty()) throw std::runtime_error("empty");
    node_base *p = head->prev;
    erase_node(p);
    delete static_cast<node*>(p);
  }
  void push_front(const T &value) {
    node *n = new node(value);
    insert_node(head->next, n);
  }
  void pop_front() {
    if (empty()) throw std::runtime_error("empty");
    node_base *p = head->next;
    erase_node(p);
    delete static_cast<node*>(p);
  }
};

} // namespace sjtu

#endif // SJTU_LIST_HPP