test: tests/test_egraph
	./tests/test_egraph

tests/test_egraph: tests/test_egraph.cpp egraphs.hpp
	clang++ -o tests/test_egraph tests/test_egraph.cpp
