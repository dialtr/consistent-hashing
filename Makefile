CXX=g++
CXXFLAGS=-Wall -Werror -g -std=c++11

.PHONY:
all: demo

.PHONY:
clean:
	-rm -f main *.o

demo: main.o router.o
	$(CXX) $(CXXFLAGS) -o demo $^

.PHONY:
format:
	clang-format -style=Google -i main.cc

.cc.o:
	$(CXX) $(CXXFLAGS) -c $<
