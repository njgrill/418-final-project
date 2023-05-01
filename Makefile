OUTPUTDIR := bin/

CFLAGS := -std=c++14 -fvisibility=hidden -lpthread -O2 -fopenmp

SOURCES := src/Sudoku-Generator/*.cpp src/Sudoku-Solver/*.cpp src/Sudoku-Validator/*.cpp src/*.cpp
# HEADERS := src/Sudoku-Generator/*.h src/Sudoku-Solver/*.h src/Sudoku-Validator/*.h src/*.h
HEADERS := src/*.h
SOURCES_FOR_ALL := src/SudokuFrame.cpp

.SUFFIXES:
.PHONY: all clean

all: sudoku-generator sudoku-solver-seqBF sudoku-solver-seqPat sudoku-solver-prlBF sudoku-solver sudoku-validator

sudoku-generator: $(HEADERS) $(SOURCES_FOR_ALL) src/Sudoku-Generator/sudoku-generator.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Generator/sudoku-generator.cpp

sudoku-solver-seqBF: $(HEADERS) $(SOURCES_FOR_ALL) src/Sudoku-Solver/sudoku-solver-seqBF.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Solver/sudoku-solver-seqBF.cpp

sudoku-solver-seqPat: $(HEADERS) $(SOURCES_FOR_ALL) src/Sudoku-Solver/sudoku-solver-seqPat.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Solver/sudoku-solver-seqPat.cpp

sudoku-solver-prlBF: $(HEADERS) $(SOURCES_FOR_ALL) src/Sudoku-Solver/sudoku-solver-prlBF.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Solver/sudoku-solver-prlBF.cpp

sudoku-solver: $(HEADERS) $(SOURCES_FOR_ALL) src/Sudoku-Solver/sudoku-solver.cpp
	$(CXX) -o $@ $(CFLAGS) src/Sudoku-Solver/sudoku-solver.cpp

sudoku-validator: $(HEADERS) $(SOURCES_FOR_ALL) src/Sudoku-Validator/*.cpp
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
