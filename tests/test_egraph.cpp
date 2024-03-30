// Copyright (c) 2024 Can Joshua Lehmann

#include "../egraphs.hpp"

#undef assert
#include "../../unittest.cpp/unittest.hpp"

enum class NodeKind {
  F, G, H,
  X, Y, Z, A, B, C
};

const char* NODE_KIND_NAMES[] = {
  "F", "G", "H",
  "X", "Y", "Z", "A", "B", "C"
};

std::ostream& operator<<(std::ostream& stream, const NodeKind& node_kind) {
  stream << NODE_KIND_NAMES[(size_t)node_kind];
  return stream;
}

int main() {
  using Node = egraphs::EGraph<NodeKind>::Node;
  using EClass = egraphs::EGraph<NodeKind>::EClass;
  
  unittest::Test("Hashcons").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    unittest_assert(e_graph.node(NodeKind::X) == e_graph.node(NodeKind::X));
    unittest_assert(e_graph.node(NodeKind::Y) != e_graph.node(NodeKind::X));
    
    Node* a = nullptr;
    Node* b = nullptr;
    
    a = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    });
    b = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    });
    unittest_assert(a == b);
    
    a = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    });
    b = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    });
    unittest_assert(a != b);
    
    a = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    });
    b = e_graph.node(NodeKind::G, {
      e_graph.node(NodeKind::X)
    });
    unittest_assert(a != b);
    
    a = e_graph.node(NodeKind::H, {
      e_graph.node(NodeKind::X),
      e_graph.node(NodeKind::Y)
    });
    b = e_graph.node(NodeKind::H, {
      e_graph.node(NodeKind::X),
      e_graph.node(NodeKind::Y)
    });
    unittest_assert(a == b);
    
    a = e_graph.node(NodeKind::H, {
      e_graph.node(NodeKind::X),
      e_graph.node(NodeKind::Y)
    });
    b = e_graph.node(NodeKind::H, {
      e_graph.node(NodeKind::X)
    });
    unittest_assert(a != b);
    
  });
  
  unittest::Test("Transitive").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    unittest_assert(e_graph.node(NodeKind::X) != e_graph.node(NodeKind::Y));
    unittest_assert(e_graph.node(NodeKind::X) != e_graph.node(NodeKind::Z));
    unittest_assert(e_graph.node(NodeKind::Y) != e_graph.node(NodeKind::Z));
    
    e_graph.merge(e_graph.node(NodeKind::X), e_graph.node(NodeKind::Y));
    
    unittest_assert(e_graph.node(NodeKind::X) == e_graph.node(NodeKind::Y));
    
    e_graph.merge(e_graph.node(NodeKind::Y), e_graph.node(NodeKind::Z));
    
    unittest_assert(e_graph.node(NodeKind::Y) == e_graph.node(NodeKind::Z));
    unittest_assert(e_graph.node(NodeKind::X) == e_graph.node(NodeKind::Z));
  });
  
  unittest::Test("Congruent (Merge Before)").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    e_graph.merge(e_graph.node(NodeKind::X), e_graph.node(NodeKind::Y));
    
    unittest_assert(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    }) == e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    }));
    
    e_graph.merge(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    }), e_graph.node(NodeKind::A));
    e_graph.merge(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    }), e_graph.node(NodeKind::B));
    
    unittest_assert(e_graph.node(NodeKind::A) == e_graph.node(NodeKind::B));
  });
  
  unittest::Test("Congruent (Merge After)").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    unittest_assert(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    }) != e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    }));
    
    e_graph.merge(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    }), e_graph.node(NodeKind::A));
    e_graph.merge(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    }), e_graph.node(NodeKind::B));
    
    e_graph.merge(e_graph.node(NodeKind::X), e_graph.node(NodeKind::Y));
    
    unittest_assert(e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    }) == e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    }));
    
    unittest_assert(e_graph.node(NodeKind::A) == e_graph.node(NodeKind::B));
    
  });
  
  unittest::Test("Congruent (Merge After, 2 Levels)").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    unittest_assert(
      e_graph.node(NodeKind::G, {
        e_graph.node(NodeKind::F, {
          e_graph.node(NodeKind::X)
        })
      })
      !=
      e_graph.node(NodeKind::G, {
        e_graph.node(NodeKind::F, {
          e_graph.node(NodeKind::Y)
        })
      })
    );
    
    e_graph.merge(e_graph.node(NodeKind::G, {
      e_graph.node(NodeKind::F, {
        e_graph.node(NodeKind::X)
      })
    }), e_graph.node(NodeKind::A));
    e_graph.merge(e_graph.node(NodeKind::G, {
      e_graph.node(NodeKind::F, {
        e_graph.node(NodeKind::Y)
      })
    }), e_graph.node(NodeKind::B));
    
    e_graph.merge(e_graph.node(NodeKind::X), e_graph.node(NodeKind::Y));
    
    unittest_assert(
      e_graph.node(NodeKind::G, {
        e_graph.node(NodeKind::F, {
          e_graph.node(NodeKind::X)
        })
      })
      ==
      e_graph.node(NodeKind::G, {
        e_graph.node(NodeKind::F, {
          e_graph.node(NodeKind::Y)
        })
      })
    );
    
    unittest_assert(e_graph.node(NodeKind::A) == e_graph.node(NodeKind::B));
  });
  
  
  return 0;
}
