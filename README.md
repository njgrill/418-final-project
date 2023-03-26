# Parallel Sudoku Solver
#### By: Kevin Song and Nicholas Grill

### URL
[https://github.com/njgrill/418-final-project](https://github.com/njgrill/418-final-project)

### Summary
We plan to use a multi-stage algorithm, where we first determine which numbers are valid for each square (based on patterns found in the initially given field), then rely on a brute-force algorithm to determine a full solution. We plan to use OpenMP to indicate parallel tasks on the usual multi-core GHC machines.


### Challenge
The challenge lies in the parallelization of the brute-force algorithm; most sequential solvers rely on backtracking, which relies on recursively calling the backtracking function and determining if the next square has any valid numbers. If a square has no valid numbers, the algorithm returns to the previous square and either a) increments the square’s number to the next valid value and continues, or b) returns to the previous square if no other valid values remain. This often results in the algorithm backtracking significant distances, creating dependencies which could become a major bottleneck for parallelization.
Furthermore, it is likely that we will need to prevent workers from working on the same tasks or tasks that have been proven to be unproductive by other tasks. This leads to different design choices revolving around synchronization/communication between threads.
