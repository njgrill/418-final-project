# Parallel Sudoku Solver
#### By: Kevin Song and Nicholas Grill

### URL
[https://github.com/njgrill/418-final-project](https://github.com/njgrill/418-final-project)

### Summary
We plan to use a multi-stage algorithm, where we first determine which numbers are valid for each square (based on patterns found in the initially given field), then rely on a brute-force algorithm to determine a full solution. We plan to use OpenMP to indicate parallel tasks on the usual multi-core GHC machines.

### Background
Sudoku is a game where a player attempts to fill a 9x9 grid with numbers from 1 to 9 so that each row, column, and 3x3 subgrid contain numbers 1 through 9 only once, without repetition. The grid contains some initial, unchangeable values so that the puzzle has limited solutions. Many people enjoy solving these puzzles, and there exist many solver algorithms as well. The algorithm we wish to use would first use patterns in the initial grid to either a) determine the exact value of certain squares, or b) narrow down the possible values for that square (ex: using a row of 8 filled values to determine the last unknown value). After exhausting the patterns, we plan to use a brute-force algorithm to fill each remaining unknown square (starting from the top left of the puzzle, and moving downwards and rightwards) with possible values, and “backtrack” to rewrite previously filled values if impossible grid combinations occur.

Brute-force sudoku solver algorithms often rely on testing grid values one at a time, so introducing parallelization methods could have the potential for major speedup improvements. Furthermore, the pattern-recognition phase could be parallelized so that each square is evaluated and updated in parallel.

### Challenge
The challenge lies in the parallelization of the brute-force algorithm; most sequential solvers rely on backtracking, which relies on recursively calling the backtracking function and determining if the next square has any valid numbers. If a square has no valid numbers, the algorithm returns to the previous square and either a) increments the square’s number to the next valid value and continues, or b) returns to the previous square if no other valid values remain. This often results in the algorithm backtracking significant distances, creating dependencies which could become a major bottleneck for parallelization.

Another potential challenge may arise during the pattern-filling phase. If a pattern is recognized and leads to a square’s value being determined, the algorithm could be re-run with the newly determined value to see if a new pattern appears. However, if this step is parallelized, it may be difficult to determine the most efficient way to communicate changes to the grid to other tasks.
 
Furthermore, it is likely that we will need to prevent workers from working on the same tasks or tasks that have been proven to be unproductive by other tasks. This leads to different design choices revolving around synchronization/communication between threads.

### Resources
We plan to find and use a sequential sudoku solver written in C++ as a code base. We will modify the solver to follow our proposed algorithm, then begin implementing parallelization improvements to the solver.

### Goals & Deliverables
Plan to Achieve:
- Working parallel sudoku solver
- Improved speed-up over the sequential version of our proposed algorithm.
- Comparisons between solve times of sudoku grids with different difficulties.
- Comparison between speedup differences due to changes in algorithm parameter values.

Hope to Achieve
- Visual representation of the state of the grid after each step of the parallelized solver
- Comparison of significantly different parallelization strategies of the same base algorithm

### Platform Choice
We plan to use the 8-core GHC machines to test, run, and evaluate our project. Our project will be written in C++, using OpenMP to mark and perform parallelization.

### Schedule
- Week 1 - Apr 3 - Apr 7: 
  - Find and edit code base to follow our proposed initial algorithm. Ensure evaluation method works. Procure several test sudoku grids with different difficulties.
- Week 2 - Apr 10 - Apr 14: 
  - Begin parallelizing brute force section as well as pattern-based solving.
- Week 3 - Apr 17 - Apr 21: 
  - Complete parallelized algorithm, write milestone report.
  - Apr 19: Project Milestone Report (due 9am)
- Week 4 - Apr 24 - Apr 28:
  - Improve parallelized algorithm, document and compare between versions.
    - Distributed task queue
- Week 5 - May 1 - May 5: 
  - Create final report and poster.
  - May 4: Final Project Report (due 11:59pm)
  - May 5: Project Poster Session
