CXX=g++
CXXFLAGS=-Wall -Werror -g -std=c++11

.PHONY:
all: main

.PHONY:
clean:
	-rm -f main *.o

.PHONY:
format:
	clang-format -style=Google -i main.cc

main: main.o router.o
	$(CXX) $(CXXFLAGS) -o main $^

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<
