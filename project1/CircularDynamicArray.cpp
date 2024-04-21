#include <cstdlib>
#include <iostream>
#include <sys/select.h>

template <typename T> class CircularDynamicArray {
public:
  CircularDynamicArray() : m_capacity(2), m_length(0), m_start(0) {
    m_data = new T[m_capacity];
  }

  CircularDynamicArray(int size)
      : m_capacity(size), m_length(size), m_start(0) {
    m_data = new T[m_capacity];
  }

  CircularDynamicArray(const CircularDynamicArray &old)
      : m_capacity(old.m_capacity), m_length(old.m_length),
        m_start(old.m_start) {
    m_data = new T[m_capacity];

    for (int i = 0; i < m_length; i++) {
      int index = internal_index(i);
      m_data[index] = old.m_data[index];
    }
  }

  CircularDynamicArray &operator=(const CircularDynamicArray &rhs) {
    if (this == &rhs) {
      return *this;
    }

    delete[] m_data;

    m_start = rhs.m_start;
    m_length = rhs.m_length;
    m_capacity = rhs.m_capacity;
    m_data = new T[m_capacity];

    for (int i = 0; i < m_length; i++) {
      int index = internal_index(i);
      m_data[index] = rhs.m_data[index];
    }

    return *this;
  }

  ~CircularDynamicArray() { delete[] m_data; }

  T &operator[](int i) {
    if (i >= m_length) {
      std::cout << "invalid index " << i << " when the length is " << m_length
                << std::endl;
      return dummy;
    }

    int index = internal_index(i);
    return m_data[index];
  }

  void addEnd(T v) {
    if (m_length == m_capacity) {
      resize(m_capacity * 2);
    }

    int index = (m_start + m_length) % m_capacity;
    m_data[index] = v;
    m_length += 1;
  }

  void addFront(T v) {
    if (m_length == m_capacity) {
      resize(m_capacity * 2);
    }

    int index = (m_start - 1) % m_capacity;
    if (index == -1) {
      index = m_capacity - 1;
    }
    m_data[index] = v;
    m_start = index;
    m_length += 1;
  }

  void delEnd() {
    m_length -= 1;
    if (m_length == 0) {
      return;
    }

    if (m_length != 0 && (m_capacity / m_length) >= 4) {
      resize(m_capacity / 2);
    }
  }

  void delFront() {
    m_start = (m_start + 1) % m_capacity;
    m_length -= 1;
    if (m_length == 0) {
      return;
    }

    if ((m_capacity / m_length) == 4) {
      resize(m_capacity / 2);
    }
  }

  int length() const { return m_length; }
  int capacity() const { return m_capacity; }

  void clear() {
    delete[] m_data;

    m_data = new T[2];
    m_capacity = 2;
    m_length = 0;
    m_start = 0;
  }

  T QSelect(int k) { return select(0, m_length - 1, k - 1); }

  T select(uint left, uint right, int k) {
    if (left == right) {
      return index(left);
    }
    uint pivot_index = left + std::rand() % (right - left + 1);
    pivot_index = partition(left, right, pivot_index);

    if (k == pivot_index) {
      return index(k);
    } else if (k < pivot_index) {
      return select(left, pivot_index - 1, k);
    } else {
      return select(pivot_index + 1, right, k);
    }
  }

  uint partition(uint left, uint right, uint pivot_index) {
    T pivot_value = index(pivot_index);
    swap(pivot_index, right);

    uint store_index = left;
    for (uint i = left; i < right; i++) {
      if (index(i) < pivot_value) {
        swap(store_index, i);
        store_index += 1;
      }
    }

    swap(right, store_index);
    return store_index;
  }

  void swap(uint lhs, uint rhs) {
    T temp = index(lhs);
    index(lhs) = index(rhs);
    index(rhs) = temp;
  }

  void Sort() {
    CircularDynamicArray<T> temp(*this);
    merge_sort(temp, *this, 0, m_length);
  }

  void merge_sort(CircularDynamicArray &A, CircularDynamicArray &B, uint start,
                  uint end) {
    if (end - start <= 1) {
      return;
    }

    uint middle = start + (end - start) / 2;

    merge_sort(B, A, start, middle);
    merge_sort(B, A, middle, end);
    merge(A, B, start, middle, end);
  }

  void merge(CircularDynamicArray &A, CircularDynamicArray &B, uint start,
             uint middle, uint end) {
    uint i = start;
    uint i_len = middle;

    uint j = middle;
    uint j_len = end;

    for (uint k = start; k < end; k++) {
      if (i < i_len && (j >= j_len || A[i] <= A[j])) {
        B[k] = A[i];
        i += 1;
      } else {
        B[k] = A[j];
        j += 1;
      }
    }
  }

  int linearSearch(T e) const {
    for (int i = 0; i < m_length; i++) {
      int index = internal_index(i);
      if (m_data[index] == e) {
        return i;
      }
    }
    return -1;
  }

  int binSearch(T e) const {
    int left = 0;
    int right = m_length - 1;

    while (left <= right) {
      int m = (left + right) / 2;
      int index = internal_index(m);
      if (m_data[index] < e) {
        left = m + 1;
      } else if (m_data[index] > e) {
        right = m - 1;
      } else {
        return m;
      }
    }
    return -1;
  }

private:
  int internal_index(int i) const { return (m_start + i) % m_capacity; }

  T &index(int i) { return m_data[internal_index(i)]; }

  void resize(int size) {
    T *old = m_data;
    m_data = new T[size];
    for (int i = 0; i < m_length; i++) {
      m_data[i] = old[(m_start + i) % m_capacity];
    }
    delete[] old;

    m_capacity = size;
    m_start = 0;
  }

private:
  T *m_data;
  int m_capacity;
  int m_length;
  int m_start;

  T dummy;
};