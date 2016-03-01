CXX=gcc
CXXFLAGS=-g -std=c11 -Wall -pedantic 
BIN=final

SRC=$(wildcard *.c)
OBJ=$(SRC:%.cpp=%.o)

all: $(BIN)
	./$(BIN)

$(BIN): $(BIN).c
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q
	$(CXX) $(CXXFLAGS) $(BIN).c -o $(BIN)

.PHONY : clean
clean:
	rm -- $(BIN) tags
#	rm -r test.dSYM
