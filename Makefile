OUTPUTDIR := bin/

CFLAGS := -std=c++14 -fvisibility=hidden -lpthread -O2 -fopenmp

SOURCES := src/Sudoku-Generator/*.cpp src/Sudoku-Solver/*.cpp src/Sudoku-Validator/*.cpp
HEADERS := src/Sudoku-Generator/*.h src/Sudoku-Solver/*.h src/Sudoku-Validator/*.h

.SUFFIXES:
.PHONY: all clean

all: sudoku-generator sudoku-solver sudoku-validator

sudoku-generator: src/Sudoku-Generator/*.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Generator/*.cpp

sudoku-solver: src/Sudoku-Solver/*.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Solver/*.cpp

sudoku-validator: src/Sudoku-Validator/*.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Validator/*.cpp

format:
	clang-format -i $(SOURCES) $(HEADERS)

clean:
	rm -rf ./sudoku-*

FILES = src/Sudoku-Generator/*.cpp \
		src/Sudoku-Solver/*.cpp \
		src/Sudoku-Validator/*.cpp \
		src/Sudoku-Generator/*.h \
		src/Sudoku-Solver/*.h \
		src/Sudoku-Validator/*.h

handin.tar: $(FILES)
	tar cvf handin.tar $(FILES)
