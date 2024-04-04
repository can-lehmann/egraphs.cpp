test: tests/test_egraph
	./tests/test_egraph

tests/test_egraph: tests/test_egraph.cpp egraphs.hpp
	clang++ -g -o tests/test_egraph tests/test_egraph.cpp
