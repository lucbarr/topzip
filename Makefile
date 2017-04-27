CXX = g++
SRC = huffman.cpp
EXEC = topzip.out
FLAGS = --std=c++11

all:
	$(CXX) $(SRC) $(FLAGS) -o $(EXEC)

