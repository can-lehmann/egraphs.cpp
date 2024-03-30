// Copyright (c) 2024 Can Joshua Lehmann

#include <iostream>

#include "../egraphs.hpp"

enum class NodeKind {
  False, True,
  And, Or, Not
};

const char* NODE_KIND_NAMES[] = {
  "False", "True",
  "And", "Or", "Not"
};

std::ostream& operator<<(std::ostream& stream, const NodeKind& node_kind) {
  stream << NODE_KIND_NAMES[(size_t)node_kind];
  return stream;
}

int main() {
  using Node = egraphs::EGraph<NodeKind>::Node;
  using EClass = egraphs::EGraph<NodeKind>::EClass;
  
  egraphs::EGraph<NodeKind> e_graph;
  
  Node* node = e_graph.node(NodeKind::And, {
    e_graph.node(NodeKind::True),
    e_graph.node(NodeKind::Not, {
      e_graph.node(NodeKind::False)
    })
  });
  
  Node* a = e_graph.node(NodeKind::Not, {
    e_graph.node(NodeKind::False)
  });
  
  Node* b = e_graph.node(NodeKind::True);
  
  e_graph.merge(b, a);
  e_graph.merge(node, b);
  
  e_graph.save_dot("graph.gv");
  
  return 0;
}
