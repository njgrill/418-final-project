# Usage
To make an individual program:

`make [sudoku-[solver/generator/validtor]]`

To make all:

`make`

To run a program, make sure you are in `418-final-project` and run:

`./sudoku-[solver/generator/validator]`

For `./sudoku-solver` and `./sudoku-validator`, they read from `std::cin` the file to solve or validate. Example usage would be:

`./sudoku-solver < inputs/unsolved9x9.txt`

Input files have the format `puzzleLength` and then the puzzle separated by spaces and new lines. For `./sudoku-solver`, blank cells are represented by `0`. An example is shown below:
```
9
0 0 0 0 0 0 6 8 0
0 0 0 0 7 3 0 0 9
3 0 9 0 0 0 0 4 5
4 9 0 0 0 0 0 0 0
8 0 3 0 5 0 9 0 2
0 0 0 0 0 0 0 3 6
9 6 0 0 0 0 3 0 8
7 0 0 6 8 0 0 0 0
0 2 8 0 0 0 0 0 0
```
`./sudoku-solver` also accepts an argument for an output file, so to solve a sample 9x9 grid, one could do:

`./sudoku-solver outputs/solved9x9.txt < inputs/unsolved9x9.txt`

The default output if no argument is given is `outputs/output.txt`