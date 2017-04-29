CXX = g++
SRC = main.cpp
EXEC = topzip
FLAGS = --std=c++11

all:
	$(CXX) $(SRC) $(FLAGS) -o $(EXEC)

