// Copyright (c) 2024 Can Joshua Lehmann

#include <iostream>
#include <variant>

#include "../egraphs.hpp"

enum class NodeKind {
  Constant, Variable,
  And, Or, Not
};

const char* NODE_KIND_NAMES[] = {
  "Constant", "Variable",
  "And", "Or", "Not"
};

std::ostream& operator<<(std::ostream& stream, const NodeKind& node_kind) {
  stream << NODE_KIND_NAMES[(size_t)node_kind];
  return stream;
}

class NodeData {
private:
  NodeKind _kind;
  std::variant<bool, std::string> _value;
public:
  NodeData(const NodeKind& kind): _kind(kind) {}
  NodeData(bool value): _kind(NodeKind::Constant), _value(value) {}
  NodeData(const std::string& value): _kind(NodeKind::Variable), _value(value) {}
  NodeData(const char* value): _kind(NodeKind::Variable), _value(std::string(value)) {}
  
  NodeKind kind() const { return _kind; }
  
  inline bool constant() const { return std::get<bool>(_value); }
  inline std::string variable() const { return std::get<std::string>(_value); }
  
  bool operator==(const NodeData& other) const {
    if (_kind == other._kind) {
      switch (_kind) {
        case NodeKind::Constant:
          return std::get<bool>(_value) == std::get<bool>(other._value);
        case NodeKind::Variable:
          return std::get<std::string>(_value) == std::get<std::string>(other._value);
        default: return true;
      }
    }
    return false;
  }
  
  bool operator!=(const NodeData& other) const { return !(*this == other); }
};

template <>
struct std::hash<NodeData> {
  size_t operator()(const NodeData& data) const {
    size_t hash = std::hash<NodeKind>()(data.kind());
    switch (data.kind()) {
      case NodeKind::Constant:
        hash ^= std::hash<bool>()(data.constant()) << 17;
      break;
      case NodeKind::Variable:
        hash ^= std::hash<std::string>()(data.variable());
      break;
      default: break;
    }
    return hash;
  }
};

std::ostream& operator<<(std::ostream& stream, const NodeData& data) {
  switch (data.kind()) {
    case NodeKind::Constant: stream << (data.constant() ? "true" : "false"); break;
    case NodeKind::Variable: stream << data.variable(); break;
    default: stream << data.kind(); break;
  }
  return stream;
}

int main() {
  using EGraph = egraphs::EGraph<NodeKind, NodeData>;
  using Node = EGraph::Node;
  using EClass = EGraph::EClass;
  
  EGraph e_graph;
  
  Node* extraction_root = e_graph.node(NodeKind::Not, {
    e_graph.node(NodeKind::And, {
      e_graph.node("x"),
      e_graph.node(NodeKind::Not, {
        e_graph.node("x")
      })
    })
  });
  
  std::deque<std::pair<Node*, Node*>> queue;
  do {
    for (Node* root : e_graph.roots()) {
      for (Node* node : root->e_class()) {
        switch (node->data().kind()) {
          case NodeKind::Not:
            for (Node* and_node : node->at(0)->e_class().match(NodeKind::And)) {
              queue.emplace_back(
                node,
                e_graph.node(NodeKind::Or, {
                  e_graph.node(NodeKind::Not, {and_node->at(0)}),
                  e_graph.node(NodeKind::Not, {and_node->at(1)})
                })
              );
            }
            for (Node* or_node : node->at(0)->e_class().match(NodeKind::Or)) {
              queue.emplace_back(
                node,
                e_graph.node(NodeKind::And, {
                  e_graph.node(NodeKind::Not, {or_node->at(0)}),
                  e_graph.node(NodeKind::Not, {or_node->at(1)})
                })
              );
            }
            for (Node* not_node : node->at(0)->e_class().match(NodeKind::Not)) {
              queue.emplace_back(
                node,
                not_node->at(0)
              );
            }
            for (Node* constant_node : node->at(0)->e_class().match(NodeKind::Constant)) {
              queue.emplace_back(
                node,
                e_graph.node(!constant_node->data().constant())
              );
            }
          break;
          case NodeKind::And:
            queue.emplace_back(node, e_graph.node(NodeKind::And, {
              node->at(1),
              node->at(0)
            }));
            if (node->at(0)->e_class().match(NodeData(false)).not_empty()) {
              queue.emplace_back(node, e_graph.node(false));
            }
            if (node->at(0)->e_class().match(NodeData(true)).not_empty()) {
              queue.emplace_back(node, node->at(1));
            }
            if (node->at(0) == node->at(1)) {
              queue.emplace_back(node, node->at(0));
            }
            for (Node* not_node : node->at(0)->e_class().match(NodeKind::Not)) {
              if (not_node->at(0) == node->at(1)) {
                queue.emplace_back(node, e_graph.node(false));
              }
            }
          break;
          case NodeKind::Or:
            queue.emplace_back(node, e_graph.node(NodeKind::Or, {
              node->at(1),
              node->at(0)
            }));
            if (node->at(0)->e_class().match(NodeData(true)).not_empty() ) {
              queue.emplace_back(node, e_graph.node(true));
            }
            if (node->at(0)->e_class().match(NodeData(false)).not_empty()) {
              queue.emplace_back(node, node->at(1));
            }
            if (node->at(0) == node->at(1)) {
              queue.emplace_back(node, node->at(0));
            }
            for (Node* not_node : node->at(0)->e_class().match(NodeKind::Not)) {
              if (not_node->at(0) == node->at(1)) {
                queue.emplace_back(node, e_graph.node(true));
              }
            }
          break;
        }
      }
    }
    std::cout << queue.size() << std::endl;
  } while(e_graph.merge(queue));
  
  e_graph.save_dot("graph.gv");
  
  EGraph::Extracted extracted = e_graph.extract();
  e_graph.save_dot("extracted.gv", extracted, extraction_root);
  
  return 0;
}
