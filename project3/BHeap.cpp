#include <cassert>
#include <cmath>
#include <iostream>

template <typename T> class Node {
public:
  Node(T value)
      : m_child(nullptr), m_right(this), m_left(this), m_value(value),
        m_height(0), m_size(1) {}

  Node(const Node &old)
      : m_child(nullptr), m_right(this), m_left(this), m_value(old.m_value),
        m_height(old.m_height), m_size(old.m_size) {

    Node<T> *current = old.m_right;
    while (current != &old) {
      Node<T> *next = current->m_right;

      Node<T> *new_node = new Node<T>(current->m_value);
      new_node->m_height = current->m_height;
      new_node->m_size = current->m_size;
      new_node->m_left = this->m_left;
      new_node->m_right = this;

      new_node->m_right->m_left = new_node;
      new_node->m_left->m_right = new_node;

      if (current->m_child != nullptr) {
        new_node->m_child = new Node<T>(*current->m_child);
      }

      current = next;
    }
  }

  Node &operator=(const Node &rhs) {
    if (this == &rhs) {
      return *this;
    }

    // Delete prior chain
    this->m_left->m_right = this->m_right;
    this->m_right->m_left = this->m_left;
    delete this->m_child;
    delete this->m_left;

    // Init defaults before construction
    this->m_child = nullptr;
    this->m_right = this;
    this->m_left = this;
    this->m_value = rhs.m_value;

    this->m_height = rhs.m_height;
    this->m_size = rhs.m_size;

    Node<T> *current = rhs.m_right;
    while (current != &rhs) {
      Node<T> *next = current->m_right;

      Node<T> *new_node = new Node<T>(current->m_value);
      new_node->m_height = current->m_height;
      new_node->m_size = current->m_size;
      new_node->m_left = this->m_left;
      new_node->m_right = this;

      new_node->m_right->m_left = new_node;
      new_node->m_left->m_right = new_node;

      if (current->m_child != nullptr) {
        new_node->m_child = new Node<T>(*current->m_child);
      }

      current = next;
    }
  }

  ~Node() {
    if (this->m_child) {
      delete this->m_child;
    }

    Node<T> *current = this->m_left;
    while (current != this) {
      Node<T> *next = current->m_left;

      current->m_left = current;
      current->m_right = current;
      delete current;

      current = next;
    }
  }

  void print() const {
    std::cout << m_value << " ";

    if (this->m_child) {
      Node<T> *current = this->m_child;
      do {
        Node<T> *next = current->m_right;
        current->print();
        current = next;
      } while (current != this->m_child);
    }
  }

  void dump(int depth) const {
    for (int i = 0; i < depth; i++) {
      std::cout << " ";
    }
    std::cout << m_value << "\n";

    if (this->m_child) {
      Node<T> *current = this->m_child;
      do {
        Node<T> *next = current->m_right;
        current->dump(depth + 1);
        current = next;
      } while (current != this->m_child);
    }
  }

  void verify() const {
    if (this->m_child) {
      Node<T> *current = this->m_child;
      do {
        Node<T> *next = current->m_right;
        current->verify();

        assert(current->m_value >= this->m_value);

        current = next;
      } while (current != this->m_child);
    }
  }

  Node<T> *m_child;
  Node<T> *m_right;
  Node<T> *m_left;
  T m_value;

  int m_height;
  int m_size;
};

template <typename T> class BHeap {
public:
  BHeap() : m_min(nullptr), m_size(0) {}

  BHeap(T values[], int count) : m_min(nullptr), m_size(0) {
    for (int i = 0; i < count; i++) {
      this->insert(values[i]);
    }
    this->consolidate();
  }

  BHeap(const BHeap &old)
      : m_min(nullptr), m_size(old.m_size) {
    if (old.m_min) {
      this->m_min = new Node<T>(*old.m_min);
    }
  }

  BHeap &operator=(const BHeap &rhs) {
    if (this == &rhs) {
      return *this;
    }

    if (this->m_min) {
      delete this->m_min;
    }

    if (rhs.m_min) {
      this->m_min = new Node<T>(*rhs.m_min);
    }
    this->m_default = rhs.m_default;
    this->m_size = rhs.m_size;

    return *this;
  }

  ~BHeap() {
    if (this->m_min) {
      delete this->m_min;
    }
  }

  T peekKey() {
    if (this->m_min) {
      return this->m_min->m_value;
    } else {
      return this->m_default;
    }
  }

  T extractMin() {
    if (this->m_min == nullptr) {
      return this->m_default;
    }
    T min = this->m_min->m_value;

    // Remove minimum from list
    {
      Node<T> *left = this->m_min->m_left;
      Node<T> *right = this->m_min->m_right;

      left->m_right = right;
      right->m_left = left;
    }

    // Patch into the former place of the minimum its node's children.
    Node<T>* min_node = this->m_min;
    if (min_node->m_child) {
      Node<T> *left = min_node;

      Node<T> *current = min_node->m_child;
      Node<T> *next = min_node->m_child;

      do {
        next = current->m_right;

        {
          current->m_right = left->m_right;
          current->m_left = left;

          left->m_right->m_left = current;
          left->m_right = current;
        }

        left = current;
        current = next;
      } while (current != min_node->m_child);

      // Remove minimum
      {
        Node<T> *right = min_node->m_right;
        Node<T> *left = min_node->m_left;

        left->m_right = right;
        right->m_left = left;
      }

      this->m_min = this->m_min->m_child;
    } else if (min_node != min_node->m_right) {
      Node<T> *right = min_node->m_right;
      Node<T> *left = min_node->m_left;

      left->m_right = right;
      right->m_left = left;

      this->m_min = right;
    } else {
      this->m_min = nullptr;
    }
    min_node->m_right = min_node;
    min_node->m_left = min_node;
    min_node->m_child = nullptr;
    delete min_node;

    consolidate();

    return min;
  }

  void insert(T value) {
    m_size += 1;

    Node<T> *new_node = new Node<T>(value);
    if (this->m_min == nullptr) {
      // No roots in the heap
      this->m_min = new_node;
      return;
    }

    Node<T> *left = this->m_min->m_left;

    left->m_right = new_node;
    new_node->m_left = left;

    new_node->m_right = this->m_min;
    this->m_min->m_left = new_node;

    if (new_node->m_value < this->m_min->m_value) {
      this->m_min = new_node;
    }
  }

  void merge(BHeap<T> &heap2) {
    // Check if work needs to be done
    if (this->m_min && heap2.m_min) {
      Node<T> *current = heap2.m_min;

      do {
        Node<T> *next = current->m_right;
        Node<T> *left = this->m_min->m_left;

        left->m_right = current;
        current->m_left = left;

        current->m_right = this->m_min;
        this->m_min->m_left = current;

        current = next;
      } while (current != heap2.m_min);

      if (heap2.m_min->m_value < this->m_min->m_value) {
        this->m_min = heap2.m_min;
      }

      this->m_size += heap2.m_size;
      heap2.m_min = nullptr;
    }
  }

  void printKey() const {
    if (this->m_min) {
      Node<T> *current = this->m_min;
      do {
        std::cout << "B" << current->m_height << ":\n";
        current->print();
        std::cout << "\n\n";

        current = current->m_right;
      } while (current != this->m_min);
    }
  }

public:
  void consolidate() {
    if (this->m_min) {
      int array_size = std::log2(this->m_size) + 1;
      Node<T> **nodes = new Node<T> *[array_size];
      for (int i = 0; i < array_size; i++) {
        nodes[i] = nullptr;
      }

      Node<T> *current = this->m_min;
      Node<T> *prior = this->m_min;

      do {
        Node<T> *next = current->m_right;

        // Remove node from the doubly linked-list
        {
          Node<T> *right = current->m_right;
          Node<T> *left = current->m_left;

          left->m_right = right;
          right->m_left = left;

          current->m_right = current;
          current->m_left = current;
        }

        Node<T> *to_insert = current;
        while (nodes[to_insert->m_height] != nullptr) {
          Node<T> *parent;
          Node<T> *child;

          if (to_insert->m_value < nodes[to_insert->m_height]->m_value) {
            parent = to_insert;
            child = nodes[to_insert->m_height];
          } else {
            parent = nodes[to_insert->m_height];
            child = to_insert;
          }
          nodes[to_insert->m_height] = nullptr;

          if (parent->m_child == nullptr) {
            parent->m_child = child;
          } else {
            // For nodes with children, i.e., those with height greater than 0.

            // Find right-most child, i.e., the child with the greatest height.
            Node<T> *iter = parent->m_child;
            while (iter->m_right != parent->m_child) {
              iter = iter->m_right;
            }

            // Add child to end of children list.
            child->m_right = parent->m_child;
            child->m_left = iter;

            parent->m_child->m_left = child;
            iter->m_right = child;
          }
          parent->m_size += child->m_size;
          parent->m_height += 1;

          to_insert = parent;
        }
        nodes[to_insert->m_height] = to_insert;

        prior = current;
        current = next;
      } while (current != prior);

      this->m_min = nullptr;
      Node<T> *iter;
      for (int i = 0; i < array_size; i++) {
        if (nodes[i] != nullptr) {
          if (this->m_min == nullptr) {
            this->m_min = nodes[i];
            iter = nodes[i];
          } else {
            // Link the node to the right of the current node pointed to by
            // iter.
            Node<T> *to_insert = nodes[i];

            to_insert->m_right = iter->m_right;
            to_insert->m_left = iter;

            iter->m_right->m_left = to_insert;
            iter->m_right = to_insert;

            if (to_insert->m_value < this->m_min->m_value) {
              this->m_min = to_insert;
            }

            iter = to_insert;
          }
        }
      }

      delete[] nodes;
    }
  }

public:
  void verify() const {
    if (this->m_min) {
      this->m_min->verify();
    }
  }

  void dump() const {
    if (this->m_min) {
      Node<T> *current = this->m_min;
      do {
        current->dump(0);
        current = current->m_right;
      } while (current != this->m_min);
    } else {
      std::cout << "Heap is empty!\n";
    }
  }

private:
  Node<T> *m_min;
  int m_size;

  T m_default;
};
