#include <cassert>
#include <iostream>
#include <sys/select.h>

#include "CircularDynamicArray.cpp"

template <typename K, typename V> class Element {
public:
  Element(K key, V value) : m_values() {
    m_values.addEnd(value);
    m_key = key;
  }

public:
  CircularDynamicArray<V> m_values;
  K m_key;
};

template <typename K, typename V> class Node {
public:
  Node(K key, V value) {
    m_elements[0] = new Element<K, V>(key, value);
    m_elements[1] = nullptr;
    m_elements[2] = nullptr;
    m_leaf = true;
    m_count = 1;
    m_kind = 2;

    m_parent = nullptr;
    for (int i = 0; i < 4; i++) {
      m_children[i] = nullptr;
    }
  }

  Node(Element<K, V> *element) {
    m_elements[0] = element;
    m_elements[1] = nullptr;
    m_elements[2] = nullptr;
    m_leaf = true;
    m_count = element->m_values.length();
    m_kind = 2;

    m_parent = nullptr;
    for (int i = 0; i < 4; i++) {
      m_children[i] = nullptr;
    }
  }

  Node(const Node &old) {
    for (int i = 0; i < old.elements(); i++) {
      m_elements[i] = new Element<K, V>(old.m_elements[i]);
    }

    if (!old.m_leaf) {
      for (int i = 0; i < old.children(); i++) {
        m_children[i] = new Node<K, V>(old.m_children[i]);
        m_children[i]->m_parent = old.m_parent;
      }
    }

    m_count = old.m_count;
    m_kind = old.m_kind;
    m_leaf = old.m_leaf;
  }

  Node &operator==(const Node &rhs) {
    if (this == &rhs) {
      return *this;
    }

    // Delete old data
    for (int i = 0; i < this->elements(); i++) {
      delete m_elements[i];
    }

    for (int i = 0; i < this->children(); i++) {
      delete m_children[i];
    }

    // Init new data
    for (int i = 0; i < rhs.elements(); i++) {
      m_elements[i] = new Element<K, V>(rhs.m_elements[i]);
    }

    if (!rhs.m_leaf) {
      for (int i = 0; i < rhs.children(); i++) {
        m_children[i] = new Node<K, V>(rhs.m_children[i]);
        m_children[i]->m_parent = rhs.m_parent;
      }
    }

    m_count = rhs.m_count;
    m_kind = rhs.m_kind;
    m_leaf = rhs.m_leaf;

    return *this;
  }

  ~Node() {
    for (int i = 0; i < this->elements(); i++) {
      delete m_elements[i];
    }

    for (int i = 0; i < this->children(); i++) {
      delete m_children[i];
    }
  }

  int elements() { return m_kind - 1; }
  int children() {
    if (!m_leaf) {
      return m_kind;
    }
    return 0;
  }

  void preorder() {
    for (int i = 0; i < this->elements(); i++) {
      std::cout << m_elements[i]->m_key << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < this->children(); i++) {
      m_children[i]->preorder();
    }
  }

  void inorder() {
    if (m_leaf) {
      for (int i = 0; i < this->elements(); i++) {
        std::cout << m_elements[i]->m_key << " ";
      }
    } else {
      for (int i = 0; i < this->elements(); i++) {
        m_children[i]->inorder();
        for (int j = 0; j < m_elements[i]->m_values.length(); j++) {
          std::cout << m_elements[i]->m_key << " ";
        }
      }
      m_children[this->children() - 1]->inorder();
    }
  }

  void postorder() {
    for (int i = 0; i < this->children(); i++) {
      m_children[i]->preorder();
    }
    for (int i = 0; i < this->elements(); i++) {
      std::cout << m_elements[i]->m_key << " ";
    }
    std::cout << std::endl;
  }

  void dump_int(int depth) {
    for (int i = 0; i < depth; i++) {
      std::cout << " ";
    }
    std::cout << "(" << m_count << "): ";

    for (int i = 0; i < this->elements(); i++) {
      std::cout << this->m_elements[i]->m_key << ", ";
    }
    std::cout << std::endl;

    for (int i = 0; i < this->children(); i++) {
      m_children[i]->dump_int(depth + 1);
    }
  }

  void verify() {
    assert(2 <= m_kind && m_kind <= 4);

    int count = 0;
    for (int i = 0; i < this->elements(); i++) {
      count += m_elements[i]->m_values.length();
    }
    if (!m_leaf) {
      for (int i = 0; i < this->children(); i++) {
        assert(this == m_children[i]->m_parent);
        count += m_children[i]->m_count;
        m_children[i]->verify();
      }
    }
    assert(count == m_count);

    for (int i = 0; i < this->elements() - 1; i++) {
      assert(m_elements[i]->m_key < m_elements[i + 1]->m_key);
    }

    for (int i = 0; i < this->elements(); i++) {
      assert(m_elements[i]->m_values.length() != 0);
    }

    for (int i = this->elements(); i < 3; i++) {
      assert(m_elements[i] == nullptr);
    }

    for (int i = this->children(); i < 4; i++) {
      assert(m_children[i] == nullptr);
    }
  }

public:
  Element<K, V> *m_elements[3];
  Node<K, V> *m_children[4];
  Node<K, V> *m_parent;

  bool m_leaf;
  int m_count;
  int m_kind;
};

template <typename K, typename V> class two4Tree {
public:
  two4Tree() : m_root(nullptr) {}

  two4Tree(K k[], V v[], int s) : m_root(nullptr) {
    for (int i = 0; i < s; i++) {
      this->insert(k[i], v[i]);
    }
  }

  two4Tree(const two4Tree &old) { m_root = new Node<K, V>(old.m_root); }

  two4Tree &operator==(const two4Tree &rhs) {
    if (this == &rhs) {
      return *this;
    }

    if (m_root) {
      delete m_root;
    }

    m_root = new Node<K, V>(rhs.m_root);

    return *this;
  }

  ~two4Tree() { delete m_root; }

  V *search(K key) {
    Node<K, V> *node = this->find_internal(key);
    if (!node) {
      return nullptr;
    }

    int i;
    for (i = 0; i < node->elements(); i++) {
      if (key <= node->m_elements[i]->m_key) {
        return &node->m_elements[i]->m_values[0];
      }
    }

    return nullptr;
  }

  void insert(K key, V value) {
    if (m_root == nullptr) {
      m_root = new Node<K, V>(key, value);
      return;
    }

    Node<K, V> *current = m_root;
    while (true) {
      current->m_count += 1;
      if (current->m_kind == 4) {
        if (current->m_parent == nullptr) {
          // If root node
          Node<K, V> *left = new Node<K, V>(current->m_elements[0]);
          left->m_children[0] = current->m_children[0];
          left->m_children[1] = current->m_children[1];
          left->m_leaf = current->m_leaf;
          left->m_parent = current;

          Node<K, V> *right = new Node<K, V>(current->m_elements[2]);
          right->m_children[0] = current->m_children[2];
          right->m_children[1] = current->m_children[3];
          right->m_leaf = current->m_leaf;
          right->m_parent = current;

          // All children either have children or don't have children
          if (!left->m_leaf) {
            left->m_children[0]->m_parent = left;
            left->m_children[1]->m_parent = left;
            left->m_count +=
                left->m_children[0]->m_count + left->m_children[1]->m_count;

            right->m_children[0]->m_parent = right;
            right->m_children[1]->m_parent = right;
            right->m_count +=
                right->m_children[0]->m_count + right->m_children[1]->m_count;
          }

          current->m_elements[0] = current->m_elements[1];
          current->m_elements[1] = nullptr;
          current->m_elements[2] = nullptr;
          current->m_children[0] = left;
          current->m_children[1] = right;
          current->m_children[2] = nullptr;
          current->m_children[3] = nullptr;
          current->m_leaf = false;
          current->m_kind = 2;
        } else {
          // If non-root node
          Node<K, V> *parent = current->m_parent;

          int i;
          for (i = 0; i < parent->children(); i++) {
            if (current == parent->m_children[i]) {
              break;
            }
          }

          // Move over parent's elements to make room for middle element of
          // current node.
          for (int j = parent->elements(); j > i; j--) {
            parent->m_elements[j] = parent->m_elements[j - 1];
          }
          parent->m_elements[i] = current->m_elements[1];
          current->m_elements[1] = nullptr;
          parent->m_kind += 1;

          Node<K, V> *right = new Node<K, V>(current->m_elements[2]);
          right->m_children[0] = current->m_children[2];
          right->m_children[1] = current->m_children[3];
          right->m_leaf = current->m_leaf;
          right->m_parent = parent;

          current->m_count = current->m_elements[0]->m_values.length();
          current->m_elements[2] = nullptr;
          current->m_children[2] = nullptr;
          current->m_children[3] = nullptr;
          current->m_kind = 2;

          if (!current->m_leaf) {
            current->m_children[0]->m_parent = current;
            current->m_children[1]->m_parent = current;

            right->m_children[0]->m_parent = right;
            right->m_children[1]->m_parent = right;

            current->m_count += current->m_children[0]->m_count;
            current->m_count += current->m_children[1]->m_count;

            right->m_count += right->m_children[0]->m_count;
            right->m_count += right->m_children[1]->m_count;
          }

          for (int j = parent->children() - 1; j > i + 1; j--) {
            parent->m_children[j] = parent->m_children[j - 1];
          }
          parent->m_children[i] = current;
          parent->m_children[i + 1] = right;

          if (key == parent->m_elements[i]->m_key) {
            parent->m_elements[i]->m_values.addEnd(value);
            return;
          } else if (key > parent->m_elements[i]->m_key) {
            current = right;
          }
          current->m_count += 1;
        }
      }

      // Find the position where the element should go.
      int i;
      for (i = 0; i < current->elements(); i++) {
        if (key <= current->m_elements[i]->m_key) {
          break;
        }
      }

      // Check if key already exists
      if (i < current->elements() && key == current->m_elements[i]->m_key) {
        current->m_elements[i]->m_values.addEnd(value);
        return;
      } else if (current->m_leaf) {
        // Move elements over
        for (int j = current->elements(); j > i; j--) {
          current->m_elements[j] = current->m_elements[j - 1];
        }

        current->m_elements[i] = new Element<K, V>(key, value);
        current->m_kind += 1;
        return;
      } else {
        current = current->m_children[i];
      }
    }
  }

  int remove(K key) {
    Node<K, V> *current = m_root;

    while (current) {
      if (current->m_parent && current->m_kind == 2) {
        adjust_(current);
      }

      // Find value or location for value
      int i = 0;
      while (i < current->elements() && key > current->m_elements[i]->m_key) {
        i++;
      }

      if (i < current->elements() && key == current->m_elements[i]->m_key) {
        Node<K, V> *bottom;
        if (current->m_elements[i]->m_values.length() == 1) {
          if (current->m_leaf) {
            delete current->m_elements[i];
            current->m_kind -= 1;

            for (int j = i; j < current->elements(); j++) {
              current->m_elements[j] = current->m_elements[j + 1];
            }
            current->m_elements[current->elements()] = nullptr;

            if (current == m_root && current->elements() == 0) {
              m_root = nullptr;
              delete current;
              return 1;
            }

            bottom = current;
          } else {
            // Find predecessor and then swap elements -> delete element from
            // that leaf.
            Node<K, V> *pred = current->m_children[i];
            while (!pred->m_leaf) {
              Node<K, V>* next = pred->m_children[pred->children() - 1];
              if (pred->m_kind == 2) {
                adjust_(pred);
              }
              pred = next;
            }
            if (pred->m_kind == 2) {
              adjust_(pred);
            }

            // The node where the element-to-be-deleted was could have been
            // moved, so find the new location.
            int j;
            current = m_root;
            while (current) {
              for (j = 0; j < current->elements(); j++) {
                if (key <= current->m_elements[j]->m_key) {
                  break;
                }
              }

              if (j < current->elements() &&
                  key == current->m_elements[j]->m_key) {
                break;
              } else {
                current = current->m_children[j];
              }
            }

            if (current != pred) {
              Element<K, V> *temp = current->m_elements[j];
              current->m_elements[j] = pred->m_elements[pred->elements() - 1];
              pred->m_elements[pred->elements() - 1] = nullptr;
              pred->m_count -= current->m_elements[j]->m_values.length();
              pred->m_kind -= 1;
              delete temp;

              bottom = pred->m_parent;
              while (bottom != current) {
                bottom->m_count -= current->m_elements[j]->m_values.length();
                bottom = bottom->m_parent;
              }

              bottom = current;
            } else {
              Element<K, V> *temp = current->m_elements[j];
              current->m_elements[j] = pred->m_elements[pred->elements() - 1];
              pred->m_elements[pred->elements() - 1] = nullptr;
              pred->m_kind -= 1;
              delete temp;

              bottom = current;
            }
          }
        } else {
          current->m_elements[i]->m_values.delFront();
          bottom = current;
        }
        while (bottom) {
          bottom->m_count -= 1;
          bottom = bottom->m_parent;
        }

        return 1;
      } else {
        current = current->m_children[i];
      }
    }

    return 0;
  }

  int rank(K key) {
    int count = 0;
    Node<K, V> *current = m_root;
    while (current) {
      int i;
      for (i = 0; i < current->elements(); i++) {
        if (key <= current->m_elements[i]->m_key) {
          break;
        }
      }

      if (!current->m_leaf) {
        for (int j = 0; j < i; j++) {
          count += current->m_children[j]->m_count;
        }
      }
      for (int j = 0; j < i; j++) {
        count += current->m_elements[j]->m_values.length();
      }

      if (i < current->elements() && key == current->m_elements[i]->m_key) {
        if (!current->m_leaf) {
          count += current->m_children[i]->m_count;
        }

        break;
      } else {
        current = current->m_children[i];
      }
    }

    return count + 1;
  }

  K select(const int pos) {
    Node<K, V> *current = m_root;

    int count = 0;
    while (current) {
      int i;
      for (i = 0; i < current->elements(); i++) {
        int new_count = count + 1;
        if (!current->m_leaf) {
          new_count += current->m_children[i]->m_count;
        }

        if (pos < new_count) {
          break;
        }
        count = new_count;
        if (pos == new_count) {
          break;
        }
      }

      if (i < current->elements() && count == pos) {
        return current->m_elements[i]->m_key;
      } else {
        current = current->m_children[i];
      }
    }

    return 0;
  }

  int duplicates(K key) {
    Node<K, V> *node = this->find_internal(key);
    if (!node) {
      return 0;
    }

    int i;
    for (i = 0; i < node->elements(); i++) {
      if (key <= node->m_elements[i]->m_key) {
        return node->m_elements[i]->m_values.length();
      }
    }

    return 0;
  }

  int size() {
    if (m_root) {
      return m_root->m_count;
    } else {
      return 0;
    }
  }

private:
  /*
  The next node (Z) on the search path is a 2-node.

    Does Z have a right sibling (X)
      Is X a 2-node
        Merge Z & X
      Otherwise
        Rotate X leftmost key from X up
    Does Z have a left sibling (Y)
      Is Y a 2-node
        Merge Z & Y
      Otherwise
        Rotate Y's rightmost key from Y up
  */

  void adjust_(Node<K, V> *node) {
    Node<K, V> *parent = node->m_parent;

    int i;
    for (i = 0; i < parent->children(); i++) {
      if (node == parent->m_children[i]) {
        break;
      }
    }

    if (parent->m_kind == 2 && parent->m_children[0]->m_kind == 2 &&
        parent->m_children[1]->m_kind == 2) {
      // Means that parent is root.
      node->m_elements[1] = parent->m_elements[0];
      node->m_kind = 4;

      if (i == 0) {
        Node<K, V> *sibling = parent->m_children[1];

        node->m_elements[2] = sibling->m_elements[0];

        node->m_children[2] = sibling->m_children[0];
        node->m_children[3] = sibling->m_children[1];

        sibling->m_elements[0] = nullptr;
        sibling->m_children[0] = nullptr;
        sibling->m_children[1] = nullptr;
        delete sibling;
      } else {
        Node<K, V> *sibling = parent->m_children[0];

        node->m_elements[2] = node->m_elements[0];
        node->m_elements[0] = sibling->m_elements[0];

        node->m_children[2] = node->m_children[0];
        node->m_children[3] = node->m_children[1];

        node->m_children[0] = sibling->m_children[0];
        node->m_children[1] = sibling->m_children[1];

        sibling->m_elements[0] = nullptr;
        sibling->m_children[0] = nullptr;
        sibling->m_children[1] = nullptr;
        delete sibling;
      }

      // Fix-up parent's child pointer
      if (parent->m_parent) {
        for (int j = 0; j < parent->m_parent->children(); j++) {
          if (parent == parent->m_parent->m_children[j]) {
            parent->m_parent->m_children[j] = node;
            break;
          }
        }
      } else {
        m_root = node;
      }

      // Fix-up parent pointers
      node->m_count = 0;
      for (int i = 0; i < node->elements(); i++) {
        node->m_count += node->m_elements[i]->m_values.length();
      }

      if (!node->m_leaf) {
        for (int j = 0; j < node->children(); j++) {
          node->m_count += node->m_children[j]->m_count;
          node->m_children[j]->m_parent = node;
        }
      }
      node->m_parent = parent->m_parent;

      parent->m_elements[0] = nullptr;
      parent->m_children[0] = nullptr;
      parent->m_children[1] = nullptr;

      delete parent;
      return;

    } else if (i + 1 < parent->children()) {
      Node<K, V> *right = parent->m_children[i + 1];

      if (right->m_kind == 2) {
        node->m_elements[0] = node->m_elements[0];
        node->m_elements[1] = node->m_parent->m_elements[i];
        node->m_elements[2] = right->m_elements[0];

        node->m_children[2] = right->m_children[0];
        node->m_children[3] = right->m_children[1];

        node->m_count += node->m_elements[1]->m_values.length();
        node->m_count += node->m_elements[2]->m_values.length();
        if (!node->m_leaf) {
          node->m_count += right->m_children[0]->m_count;
          node->m_count += right->m_children[1]->m_count;

          node->m_children[2]->m_parent = node;
          node->m_children[3]->m_parent = node;
        }

        for (int j = i; j < parent->elements() - 1; j++) {
          parent->m_elements[j] = parent->m_elements[j + 1];
        }
        parent->m_elements[parent->elements() - 1] = nullptr;

        for (int j = i + 1; j < parent->children() - 1; j++) {
          parent->m_children[j] = parent->m_children[j + 1];
        }
        parent->m_children[parent->children() - 1] = nullptr;
        parent->m_kind -= 1;

        right->m_elements[0] = nullptr;
        right->m_children[0] = nullptr;
        right->m_children[1] = nullptr;

        node->m_kind = 4;
        delete right;
        return;
      } else {
        // Prepare sibling node for new largest element
        node->m_kind += 1;

        // Move parent element down
        node->m_elements[1] = parent->m_elements[i];

        // Move the child over to the current node and the smallest element from
        // the sibling to the parent node.
        node->m_children[node->children() - 1] = right->m_children[0];
        parent->m_elements[i] = right->m_elements[0];
        right->m_kind -= 1;

        for (int j = 0; j < right->elements(); j++) {
          right->m_elements[j] = right->m_elements[j + 1];
        }
        right->m_elements[right->elements()] = nullptr;
        for (int j = 0; j < right->children(); j++) {
          right->m_children[j] = right->m_children[j + 1];
        }
        right->m_children[right->children()] = nullptr;

        // Update counts
        node->m_count += node->m_elements[1]->m_values.length();
        right->m_count -= parent->m_elements[i]->m_values.length();
        if (!node->m_leaf) {
          node->m_count += node->m_children[node->children() - 1]->m_count;
          right->m_count -= node->m_children[node->children() - 1]->m_count;

          node->m_children[node->children() - 1]->m_parent = node;
        }
        return;
      }
    } else {
      Node<K, V> *left = parent->m_children[i - 1];

      if (left->m_kind == 2) {
        node->m_elements[2] = node->m_elements[0];
        node->m_elements[1] = parent->m_elements[i - 1];
        node->m_elements[0] = left->m_elements[0];

        node->m_children[3] = node->m_children[1];
        node->m_children[2] = node->m_children[0];
        node->m_children[1] = left->m_children[1];
        node->m_children[0] = left->m_children[0];

        node->m_count += node->m_elements[0]->m_values.length();
        node->m_count += node->m_elements[1]->m_values.length();
        if (!node->m_leaf) {
          node->m_count += left->m_children[0]->m_count;
          node->m_count += left->m_children[1]->m_count;

          node->m_children[0]->m_parent = node;
          node->m_children[1]->m_parent = node;
        }

        for (int j = i; j < parent->elements(); j++) {
          parent->m_elements[j - 1] = parent->m_elements[j];
        }
        parent->m_elements[parent->elements() - 1] = nullptr;

        for (int j = i; j < parent->children(); j++) {
          parent->m_children[j - 1] = parent->m_children[j];
        }
        parent->m_children[parent->children() - 1] = nullptr;
        parent->m_kind -= 1;

        left->m_elements[0] = nullptr;
        left->m_children[0] = nullptr;
        left->m_children[1] = nullptr;

        node->m_kind = 4;
        delete left;
        return;
      } else {
        // Prepare current node for new smallest element
        node->m_elements[1] = node->m_elements[0];
        node->m_elements[0] = nullptr;

        node->m_children[2] = node->m_children[1];
        node->m_children[1] = node->m_children[0];
        node->m_children[0] = nullptr;
        node->m_kind += 1;

        // Move parent element down
        node->m_elements[0] = parent->m_elements[i - 1];

        // Move largest node from left to current
        parent->m_elements[i - 1] = left->m_elements[left->elements() - 1];
        left->m_elements[left->elements() - 1] = nullptr;

        node->m_children[0] = left->m_children[left->children() - 1];
        left->m_children[left->children() - 1] = nullptr;
        left->m_kind -= 1;

        // Update counts
        node->m_count += node->m_elements[0]->m_values.length();
        left->m_count -= parent->m_elements[i - 1]->m_values.length();
        if (!node->m_leaf) {
          node->m_count += node->m_children[0]->m_count;
          left->m_count -= node->m_children[0]->m_count;

          node->m_children[0]->m_parent = node;
        }
        return;
      }
    }
  }

  Node<K, V> *find_internal(K key) {
    Node<K, V> *current = m_root;
    while (current) {
      int i;
      for (i = 0; i < current->elements(); i++) {
        if (key <= current->m_elements[i]->m_key) {
          break;
        }
      }

      if (i < current->elements() && key == current->m_elements[i]->m_key) {
        return current;
      } else if (current->m_leaf) {
        return nullptr;
      } else {
        current = current->m_children[i];
      }
    }

    return nullptr;
  }

public:
  void preorder() {
    if (m_root) {
      m_root->preorder();
    }
  }

  void inorder() {
    if (m_root) {
      m_root->inorder();
      std::cout << std::endl;
    }
  }

  void postorder() {
    if (m_root) {
      m_root->postorder();
    }
  }

  void verify() {
    if (m_root) {
      m_root->verify();
    }
  }

  void dump() {
    if (m_root) {
      std::cout << "Tree is:" << std::endl;
      m_root->dump_int(0);
      std::cout << std::endl;
    } else {
      std::cout << "Tree is empty\n" << std::endl;
    }
  }

private:
  Node<K, V> *m_root;
};