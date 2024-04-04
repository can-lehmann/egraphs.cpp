// Copyright 2024 Can Joshua Lehmann
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
  
  unittest::Test("Match").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    Node* a = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    });
    
    Node* b = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    });
    
    Node* c = e_graph.node(NodeKind::G, {
      e_graph.node(NodeKind::X)
    });
    
    e_graph.merge(a, b);
    e_graph.merge(a, c);
    
    auto check_matches = [&](Node* node, const NodeKind& kind, size_t expected_count){
      size_t count = 0;
      for (Node* match : node->e_class().match(kind)) {
        unittest_assert(match->data() == kind);
        count++;
      }
      unittest_assert(count == expected_count);
    };
    
    check_matches(a, NodeKind::F, 2);
    check_matches(b, NodeKind::F, 2);
    check_matches(c, NodeKind::F, 2);
    
    check_matches(a, NodeKind::G, 1);
    check_matches(b, NodeKind::G, 1);
    check_matches(c, NodeKind::G, 1);
    
    check_matches(a, NodeKind::X, 0);
    check_matches(b, NodeKind::X, 0);
    check_matches(c, NodeKind::X, 0);
  });
  
  unittest::Test("Match (Collect)").run([](){
    egraphs::EGraph<NodeKind> e_graph;
    
    Node* a = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::X)
    });
    
    Node* b = e_graph.node(NodeKind::F, {
      e_graph.node(NodeKind::Y)
    });
    
    Node* c = e_graph.node(NodeKind::G, {
      e_graph.node(NodeKind::X)
    });
    
    e_graph.merge(a, b);
    e_graph.merge(a, c);
    
    auto check_matches = [&](Node* node, const NodeKind& kind, size_t expected_count){
      std::vector<Node*> matches;
      matches.insert(matches.end(),
        node->e_class().match(kind).begin(),
        node->e_class().match(kind).end()
      );
      
      unittest_assert(matches.size() == expected_count);
      for (Node* match : matches) {
        unittest_assert(match->data() == kind);
      }
    };
    
    check_matches(a, NodeKind::F, 2);
    check_matches(b, NodeKind::F, 2);
    check_matches(c, NodeKind::F, 2);
    
    check_matches(a, NodeKind::G, 1);
    check_matches(b, NodeKind::G, 1);
    check_matches(c, NodeKind::G, 1);
    
    check_matches(a, NodeKind::X, 0);
    check_matches(b, NodeKind::X, 0);
    check_matches(c, NodeKind::X, 0);
  });
  
  return 0;
}
