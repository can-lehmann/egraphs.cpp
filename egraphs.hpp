// Copyright (c) 2024 Can Joshua Lehmann

#ifndef EGRAPHS_HPP
#define EGRAPHS_HPP

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <cassert>
#include <cinttypes>

#define throw_error(Error, msg) { \
  std::ostringstream message_stream; \
  message_stream << msg; \
  throw Error(message_stream.str()); \
}

#define owned(Type) \
  Type(const Type& other) = delete; \
  Type& operator=(const Type& other) = delete;

namespace egraphs {
  class Error: public std::runtime_error {
    using std::runtime_error::runtime_error;
  };
  
  class ArenaAllocator {
  private:
    struct Arena {
      static constexpr const uintptr_t SIZE = 4 * 1024 * 1024; 
      
      uint8_t* data = nullptr;
      uintptr_t top = (uintptr_t)nullptr;
      
      explicit Arena() {
        data = new uint8_t[SIZE]; 
        top = (uintptr_t)data;
      }
      
      owned(Arena)
      
      Arena(Arena&& other):
        data(std::exchange(other.data, nullptr)),
        top(std::exchange(other.top, 0)) {}
      
      ~Arena() {
        if (data != nullptr) {
          delete[] data;
          data = nullptr;
        }
      }
      
      void* alloc(uintptr_t size, uintptr_t alignment) {
        uintptr_t offset = top + alignment - top % alignment;
        if (offset + size > (uintptr_t)data + SIZE) {
          return nullptr;
        }
        top = offset + size;
        return (void*)offset;
      }
    };
    
    std::vector<Arena> _arenas;
  public:
    ArenaAllocator() {
      _arenas.emplace_back();
    }
    
    owned(ArenaAllocator)
    
    void* alloc(size_t size, size_t alignment) {
      assert(size < Arena::SIZE);
      void* ptr = _arenas.back().alloc(size, alignment);
      if (ptr == nullptr) {
        _arenas.emplace_back();
        ptr = _arenas.back().alloc(size, alignment);
        assert(ptr != nullptr);
      }
      return ptr;
    }
    
    template <class T>
    T* alloc() {
      return (T*)alloc(sizeof(T), alignof(T));
    }
  };
  
  template <class NodeData>
  class EGraph {
  public:
    struct Node;
  private:
    template <class T>
    struct CycleRange {
      T* first = nullptr;
      T* last = nullptr;
      
      CycleRange() {}
      CycleRange(T* _first, T* _last):
        first(_first), last(_last) {}
      
      bool empty() const { return first == nullptr; }
    };
    
    struct Use {
      Node* node = nullptr;
      size_t child_index = 0;
      Use* next = nullptr;
      
      Use(Node* _node, size_t _child_index):
          node(_node), child_index(_child_index) {
        next = this;
      }
    };
    
    struct Down {
      Node* node = nullptr;
      Down* next = nullptr;
      
      Down(Node* _node): node(_node) {
        next = this;
      }
    };
    
  public:
    class EClass {
    public:
      class iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Node*;
        using difference_type = std::ptrdiff_t;
        using pointer = Node**;
        using reference = Node*&;
      private:
        Down* _initial = nullptr;
        Down* _current = nullptr;
      public:
        explicit iterator(Down* initial, Down* current):
          _initial(initial), _current(current) {}
        
        iterator& operator++() {
          if (_current->next == _initial) {
            _current = nullptr;
          } else {
            _current = _current->next;
          }
          return *this;
        }
        
        iterator& operator++(int) {
          iterator old = *this;
          ++(*this);
          return old;
        }
        
        bool operator==(const iterator& other) const {
          return _initial == other._initial &&
                 _current == other._current;
        }
        
        bool operator!=(const iterator& other) const { return !(*this == other); }
        Node* operator*() const { return _current->node; }
      };
    private:
      Node* _root = nullptr;
    public:
      EClass(Node* node): _root(node->find()) {}
      
      const Node* root() const { return _root; }
      
      iterator begin() { return iterator(_root->down, _root->down); }
      iterator end() { return iterator(_root->down, nullptr); }
    };
    
    class Node {
    private:
      friend EGraph<NodeData>;
      
      NodeData data;
      
      // Union Find
      size_t rank = 0;
      Node* up = nullptr;
      
      // If this node is a root node, uses contains a cyclic linked
      // list of all users of the node and down contains a cyclic
      // linked list of all nodes in the equivalence class.
      Use* uses = nullptr;
      Down* down;
      
      // Hashcons
      Node* next_bucket = nullptr;
      Node** prev_bucket = nullptr;
      
      // The children of the node are stored in a flexible array member
      // directly after this structure.
      size_t child_count = 0;
      Node* children[];
      
      Node(const NodeData& _data,
           Down* _down,
           size_t _child_count,
           Node** _children):
          data(_data),
          down(_down),
          child_count(_child_count) {
        
        std::copy(_children, _children + _child_count, children);
      }
      
      void insert_uses(Use* _uses) {
        if (uses == nullptr) {
          uses = _uses;
        } else {
          Use* temp = uses->next;
          uses->next = _uses->next;
          _uses->next = temp;
        }
      }
      
      // Finds the root node in the union find.
      // Performs path compression.
      Node* find() {
        Node* root = this;
        while (root->up != nullptr) {
          root = root->up;
        }
        Node* cur = this;
        while (cur->up != nullptr) {
          Node* old_up = cur->up;
          cur->up = root;
          cur = old_up;
        }
        return root;
      }
      
      // Merges (union) equivalence classes.
      // `this` and `other` must both be distinct root nodes.
      // The rank of `this` must less or equal to the rank of `other`.
      CycleRange<Use> merge_roots(Node* other) {
        assert(this != other);
        assert(rank <= other->rank);
        assert(up == nullptr);
        assert(other->up == nullptr);
        
        up = other;
        if (rank == other->rank) {
          other->rank++;
        }
        
        Down* temp = other->down->next;
        other->down->next = down->next;
        down->next = temp;
        down = nullptr;
        
        if (uses != nullptr) {
          CycleRange<Use> range(uses->next, uses);
          other->insert_uses(uses);
          uses = nullptr;
          return range;
        } else {
          return CycleRange<Use>();
        }
      }
      
      // In order to find nodes in the hashcons, we need to hash
      // and compare nodes.
      
      static size_t hash(const NodeData& data, Node** children, size_t child_count) {
        size_t hash = std::hash<NodeData>()(data);
        hash ^= std::hash<size_t>()(child_count) << 17;
        for (size_t it = 0; it < child_count; it++) {
          hash ^= std::hash<Node*>()(children[it]);
        }
        return hash;
      }
      
      size_t hash() const {
        return hash(data, (Node**)children, child_count);
      }
      
      bool eq(const NodeData& _data, Node** _children, size_t _child_count) const {
        if (child_count != _child_count || data != _data) {
          return false;
        }
        
        for (size_t it = 0; it < child_count; it++) {
          if (children[it] != _children[it]) {
            return false;
          }
        }
        
        return true;
      }
      
    public:
      
      EClass e_class() { return EClass(this); }
      
      Node** begin() { return children; }
      Node** end() { return children + child_count; }
      size_t size() { return child_count; }
      
      Node* at(size_t index) {
        if (index >= child_count) {
          throw_error(Error, "");
        }
        return children[index];
      }
      
      inline Node* operator[](size_t index) { return at(index); }
    };
    
  private:
    class Hashcons {
    private:
      Node** _data = nullptr;
      size_t _size = 0;
    public:
      Hashcons() {
        _size = 1024;
        _data = new Node*[_size]();
      }
      
      owned(Hashcons)
      
      ~Hashcons() {
        delete[] _data;
      }
      
      Node* get(const NodeData& data, Node** children, size_t child_count) {
        size_t hash = Node::hash(data, children, child_count) % _size;
        Node* node = _data[hash];
        while (node != nullptr) {
          if (node->eq(data, children, child_count)) {
            return node;
          }
          node = node->next_bucket;
        }
        return nullptr;
      }
      
      // Removes a node from the hashcons.
      // Assumes that the node is currently in the hashcons.
      void erase(Node* node) {
        assert(node->prev_bucket != nullptr);
        
        *node->prev_bucket = node->next_bucket;
        if (node->next_bucket != nullptr) {
          node->next_bucket->prev_bucket = node->prev_bucket;
        }
        node->next_bucket = nullptr;
        node->prev_bucket = nullptr;
      }
      
      // Assumes that node is not currently in the hashcons.
      void insert(Node* node) {
        assert(node->prev_bucket == nullptr);
        
        size_t hash = node->hash() % _size;
        node->next_bucket = _data[hash];
        if (_data[hash] != nullptr) {
          _data[hash]->prev_bucket = &node->next_bucket;
        }
        _data[hash] = node;
        node->prev_bucket = &_data[hash];
      }
    };
    
    Hashcons _hashcons;
    std::unordered_set<Node*> _roots;
    
    ArenaAllocator _node_allocator;
    ArenaAllocator _down_allocator;
    ArenaAllocator _use_allocator;
  public:
    EGraph() {}
    owned(EGraph)
    
    const std::unordered_set<Node*>& roots() const { return _roots; }
    
    Node* node(const NodeData& data, Node** children, size_t child_count) {
      // All children must be root nodes
      for (size_t it = 0; it < child_count; it++) {
        assert(children[it]->up == nullptr);
      }
      
      // Check if node is already in hashcons
      Node* node = _hashcons.get(data, children, child_count);
      if (node != nullptr) {
        return node->find();
      }
      
      // Node is not in hashcons -> allocate new node
      node = (Node*)_node_allocator.alloc(sizeof(Node) + sizeof(Node*) * child_count, alignof(Node));
      Down* down = _down_allocator.alloc<Down>();
      new(down) Down(node);
      new(node) Node(data, down, child_count, children);
      
      for (size_t it = 0; it < child_count; it++) {
        Use* use = _use_allocator.alloc<Use>();
        new(use) Use(node, it);
        children[it]->insert_uses(use);
      }
      
      // Insert into hashcons
      _hashcons.insert(node);
      _roots.insert(node);
      
      return node;
    }
    
    Node* node(const NodeData& data, const std::vector<Node*>& children) {
      return node(data, (Node**)(children.size() > 0 ? &children[0] : nullptr), children.size());
    }
    
    Node* node(const NodeData& data) {
      return node(data, nullptr, 0);
    }
    
    void merge(Node* a, Node* b) {
      a = a->find();
      b = b->find();
      if (a == b) {
        return;
      }
      
      Node* root = b;
      Node* child = a;
      if (root->rank < child->rank) {
        std::swap(root, child);
      }
      
      CycleRange<Use> uses = child->merge_roots(root);
      _roots.erase(child);
      
      // Update users
      if (!uses.empty()) {
        Use* use = uses.first;
        while (true) {
          _hashcons.erase(use->node);
          use->node->children[use->child_index] = root;
          _hashcons.insert(use->node);
          
          if (use == uses.last) {
            break;
          }
          use = use->next;
        }
        
      }
    }
    
    void write_dot(std::ostream& stream) const {
      std::unordered_map<Node*, size_t> ids;
      
      stream << "digraph {\n";
      stream << "compound=true;\n";
      
      for (Node* root : _roots) {
        for (Node* node : root->e_class()) {
          ids.insert({node, ids.size()});
        }
        
        stream << "subgraph cluster" << ids.at(root) << " {\n";
        for (Node* node : root->e_class()) {
          stream << "node" << ids.at(node) << " [label=\"" << node->data << "\"]\n";
        }
        stream << "}\n";
      }
      
      for (Node* root : _roots) {
        for (Node* node : root->e_class()) {
          for (Node* child : *node) {
            stream << "node" << ids.at(node) << " -> node" << ids.at(child);
            //stream << " [lhead=cluster" << ids.at(child->find()) << "]";
            stream << ";\n";
          }
        }
      }
      
      stream << "}";
    }
    
    void save_dot(const char* path) {
      std::ofstream stream;
      stream.open(path);
      if (!stream) {
        throw_error(Error, "Failed to open " << path);
      }
      write_dot(stream);
    }
    
  };
};

#undef owned
#undef throw_error

#endif
