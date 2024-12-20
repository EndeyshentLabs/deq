FLAGS ?= -O3
CXXFLAGS ?= $(FLAGS) -Wall -Wextra -std=c++20 -stdlib=libc++ -pedantic

all: deq
deq: deq.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<
