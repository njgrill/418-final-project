#include<iostream>
#include<fstream>
#include<cmath>
#include<stack>
#include<vector>
#include<unordered_set>
#include<omp.h>
#include<cassert>
#include "../timing.h"
#include "../SudokuFrame.cpp"
// #define  printToggle(...) (printf(__VA_ARGS__))
#define  printToggle(...) (0)
using namespace std;

const int NUM_PROCESSORS = 16;
const int CACHE_LINE_SIZE = 64;
const int ITERATIONS_FOR_AVG = 50;
enum solType { no_solution, local_solution, global_solution, none };

// struct paddedBool {
// 	bool val;
// 	char padding[CACHE_LINE_SIZE - (sizeof(bool) % CACHE_LINE_SIZE)];
// };

// struct paddedInt {
// 	int val;
// 	char padding[CACHE_LINE_SIZE - (sizeof(int) % CACHE_LINE_SIZE)];
// };

// struct paddedSudokuFrame {
// 	SudokuFrame val;
// 	char padding[CACHE_LINE_SIZE - (sizeof(SudokuFrame) % CACHE_LINE_SIZE)];
// };

struct sharedVals {
	bool needMoreGrids;
	bool isSolved;
	int STOP_DEPTH;
	SudokuFrame grid;
	Timer solveTimer;
};

struct paddedSharedVals {
	sharedVals vals;
	char padding[CACHE_LINE_SIZE - (sizeof(sharedVals) % CACHE_LINE_SIZE)];
};

/**
  *	Takes in the SudokuFrame object and solves the Sudoku Puzzle.
*/
class SudokuSolver{
	
private:
	SudokuFrame frame; //The frame object

	// paddedBool isSolved[NUM_PROCESSORS];
	// paddedBool needMoreGrids[NUM_PROCESSORS];
	// paddedSudokuFrame sudokuFrames[NUM_PROCESSORS];
	paddedSharedVals sharedLocalVals[NUM_PROCESSORS];
	// int STOP_DEPTH = 0;
	float totalStealTime;
	int totalStealCount;

	/**
	  *	@desc Checks if the value in the specified cell is valid or not.
	  *	@param row (int) row of the required value
	  *	@param col (int) col of the required value
	  *	@param currentValue (int) the required value
	  *	@return (bool) whether the value is valid or not in the sudoku frame
	*/
	bool cellValueValid(SudokuFrame &currFrame, int currentValue){
		int rowIter, colIter;
		int row = currFrame.row;
		int col = currFrame.col;

		//Checking if value exists in same column
		for(rowIter=0; rowIter < currFrame.gridLength; rowIter++){
			if(rowIter != row){
				if (currFrame.getCellValue(rowIter,col) == currentValue) return false;
			}
		}

		//Checking if value exists in same row
		for(colIter = 0; colIter < frame.gridLength; colIter++){
			if(colIter != col){
				if (currFrame.getCellValue(row,colIter) == currentValue) return false;
			}
		}

		//Checking if value exists in the same 3x3 square block
		if (ThreeByThreeGridValid(currFrame, currentValue) == false) return false;

		return true;
	}
	
	/**
	  *	@desc Checks if the same value is also present in the same 3x3 grid block.
	  *	@param row (int) row of the required cell
	  *	@param col (int) col of the required cell
	  *	@param currentValue (int) required value
	  *	@return (bool) whether the value is present or not
	*/
	bool ThreeByThreeGridValid(SudokuFrame &currFrame, int currentValue){
		int row = currFrame.row;
		int col = currFrame.col;

		int rowStart=(row/(int)(sqrt(currFrame.gridLength)))*(int)(sqrt(currFrame.gridLength));
		int rowEnd=rowStart+((int)(sqrt(currFrame.gridLength)) - 1);
		
		int colStart=(col/(int)(sqrt(currFrame.gridLength)))*(int)(sqrt(currFrame.gridLength));
		int colEnd=colStart+((int)(sqrt(currFrame.gridLength)) - 1);

		int rowIter, colIter;

		for(rowIter=rowStart; rowIter<=rowEnd; rowIter++){
			for(colIter=colStart; colIter<=colEnd; colIter++){
				if((rowIter != row || colIter != col) && currFrame.getCellValue(rowIter,colIter) == currentValue) return false;
			}
		}

		return true;	
	}
	
	std::vector<char> getCellPossibilities(SudokuFrame &currFrame){
		int iter = 0;
		int row = currFrame.row;
		int col = currFrame.col;

		std::vector<char> possibilities;
		std::unordered_set<char> possibilitiesSet;

		// Add all items to set initially
		for (char i = 1; i <= currFrame.gridLength; i++) {
			possibilitiesSet.insert(i);
		}

		// Removing values in same column
		for(char rowIter = 0; rowIter < currFrame.gridLength; rowIter++){
			if(rowIter != row){
				possibilitiesSet.erase(currFrame.getCellValue(rowIter,col));
			}
		}

		// Remoiving values in same row
		for(int colIter = 0; colIter < frame.gridLength; colIter++){
			if(colIter != col){
				possibilitiesSet.erase(currFrame.getCellValue(row,colIter));
			}
		}

		// Removing values in same square
		int rowStart=(row/(int)(sqrt(currFrame.gridLength)))*(int)(sqrt(currFrame.gridLength));
		int rowEnd=rowStart+((int)(sqrt(currFrame.gridLength)) - 1);
		
		int colStart=(col/(int)(sqrt(currFrame.gridLength)))*(int)(sqrt(currFrame.gridLength));
		int colEnd=colStart+((int)(sqrt(currFrame.gridLength)) - 1);

		for(int rowIter = rowStart; rowIter <= rowEnd; rowIter++){
			for(int colIter = colStart; colIter <= colEnd; colIter++){
				if (rowIter != row || colIter != col) {
					possibilitiesSet.erase(currFrame.getCellValue(rowIter,colIter));
				}
			}
		}

		for (auto elem : possibilitiesSet) {
			possibilities.push_back(elem);
		}

		return possibilities;
	}

	SudokuFrame stealGrid(SudokuFrame &currFrame) {
		SudokuFrame newFrame = SudokuFrame();

		newFrame.gridLength = currFrame.gridLength;
		newFrame.startRow = currFrame.startRow;
		newFrame.startCol = currFrame.startCol;
		newFrame.row = currFrame.row;
		newFrame.col = currFrame.col;
		newFrame.setDepth();
		newFrame.endRow = currFrame.row;
		newFrame.endCol = currFrame.col;
		newFrame.sudokuFrame.resize(newFrame.gridLength);
		newFrame.possibilitiesFrame.resize(newFrame.gridLength);
		
		for (int row = 0; row < newFrame.gridLength; row++) {
			newFrame.sudokuFrame[row].resize(newFrame.gridLength);
			newFrame.possibilitiesFrame[row].resize(newFrame.gridLength);
			for (int col = 0; col < newFrame.gridLength; col++) {			
				newFrame.sudokuFrame[row][col] = currFrame.sudokuFrame[row][col];
				int halfSize = currFrame.possibilitiesFrame[row][col].size() / 2;
				newFrame.possibilitiesFrame[row][col] = std::vector<char>(currFrame.possibilitiesFrame[row][col].begin() + halfSize, currFrame.possibilitiesFrame[row][col].end());
				currFrame.possibilitiesFrame[row][col].erase(currFrame.possibilitiesFrame[row][col].begin(), currFrame.possibilitiesFrame[row][col].begin() + halfSize);
			}
		}

		return newFrame;
	}
	
	solType parallelSolve(SudokuFrame &currFrame, int threadId) {
		
		if (sharedLocalVals[threadId].vals.isSolved) {
			// Another thread has a valid solution
			return global_solution;
		} else if (currFrame.row >= currFrame.gridLength) {
			// We have a valid solution
			return local_solution;
		}

		// Try a value in an editable cell
		int row = currFrame.row;
		int col = currFrame.col;
		if(currFrame.isEditable(row,col)) {

			// Compute new value
			if (row > currFrame.endRow || (row == currFrame.endRow && col > currFrame.endCol)) {
				currFrame.possibilitiesFrame[row][col] = getCellPossibilities(currFrame);
			}
			auto &possibilities = currFrame.possibilitiesFrame[row][col];

			// Split grid if previous thread needs; test-and-test-and-set
			// if (currFrame.getDepth() < sharedLocalVals[threadId].vals.STOP_DEPTH && sharedLocalVals[threadId].vals.needMoreGrids == true) {
			if (currFrame.getDepth() < sharedLocalVals[threadId].vals.STOP_DEPTH && sharedLocalVals[threadId].vals.needMoreGrids == true) {
				// Split grid
				// printf("%d is sending grid\n", threadId);
				auto beforeStealing = sharedLocalVals[threadId].vals.solveTimer.elapsed();
				auto newGrid = stealGrid(currFrame);
				sharedLocalVals[(threadId + NUM_PROCESSORS - 1) % NUM_PROCESSORS].vals.grid = std::move(newGrid);
				sharedLocalVals[threadId].vals.needMoreGrids = false;
				auto afterStealing = sharedLocalVals[threadId].vals.solveTimer.elapsed();
				#pragma atomic
				totalStealCount++;
				#pragma atomic
				totalStealTime += (afterStealing - beforeStealing);
				printf("Stealing up to depth %d took: %.6fs\n", currFrame.getDepth(), afterStealing - beforeStealing);
				// STOP_DEPTH++;
				// printf("STOP_DEPTH: %d\n", STOP_DEPTH);
				// printf("%d finished sending steal; changed %d needMoreGrids val to false\n", threadId, (threadId + NUM_PROCESSORS - 1) % NUM_PROCESSORS);
			}

			// Iterate over possible values
			while (possibilities.size() > 0) {
			// for(int posIter = 0; posIter < possibilities.size(); posIter++) {	
				currFrame.sudokuFrame[row][col] = possibilities.back();
				possibilities.pop_back();
				
				//We now increment the col/row values for the next recursion
				currFrame.updateRowCol();
				currFrame.findNextRowCol();
				auto res = parallelSolve(currFrame, threadId);

				// Solution found (local or global)
				if(res != no_solution){
					return res;
				}

				// Backtrack
				currFrame.findPrevRowCol();
				currFrame.sudokuFrame[row][col] = 0;
			}

			return no_solution;
		}
		else {
			// Seek first editable chunk
			currFrame.findNextRowCol();

			return parallelSolve(currFrame, threadId);
		}
	}

	// Returns how many grids were populated
	int populateGridsInit(SudokuFrame &grid, int startInit, int numLeftInit) {
		// SudokuFrame &grid = originalFrame;

		grid.findNextRowCol();
		std::vector<char> currPossibilities = getCellPossibilities(grid);
		int numLeft = numLeftInit;
		int start = startInit;

		while (currPossibilities.size() > 0 && numLeft > 0) {
			// Divide up possibilities and return
			if (numLeft <= currPossibilities.size()) {
				for (int i = 0; i < numLeft - 1; i++) {
					SudokuFrame newFrame = SudokuFrame(grid);
					newFrame.possibilitiesFrame[grid.row][grid.col] = std::vector<char>{currPossibilities.back()}; currPossibilities.pop_back();
					newFrame.endRow = grid.row;
					newFrame.endCol = grid.col;
					sharedLocalVals[start + numLeft - i - 1].vals.grid = newFrame;
					// sudokuFrames[start + numLeft - 1].val.displayFrame();
					// printf("frame %d got populated!\n", start + numLeft - i - 1);
				}
				numLeft = 1;
				SudokuFrame newFrame = SudokuFrame(grid);
				newFrame.possibilitiesFrame[grid.row][grid.col] = std::move(currPossibilities);
				newFrame.endRow = grid.row;
				newFrame.endCol = grid.col;
				sharedLocalVals[start + numLeft - 1].vals.grid = newFrame;
				// printf("frame %d got populated!\n", start + numLeft - 1);
				numLeft--;
				return (numLeftInit - numLeft);
			}

			// Set value to grid cell and recurse
			grid.sudokuFrame[grid.row][grid.col] = currPossibilities.back(); currPossibilities.pop_back();
			grid.updateRowCol();
			auto numPopulated = populateGridsInit(grid, start, numLeft - currPossibilities.size());
			start += numPopulated;
			numLeft -= numPopulated;

			// Backtrack
			grid.findPrevRowCol();
			grid.sudokuFrame[grid.row][grid.col] = 0;
		}

		return (numLeftInit - numLeft);
	}

	/**
	  *	@desc Calls the singleCellSolve() func and prints a success/fail mesg.
	  *	@param none
	  *	@return none
	*/
	SudokuFrame solve(SudokuFrame originalFrame, bool printInfo) {
		// Create final grid
		if (printInfo) printf("Starting solve...\n");
		SudokuFrame finalFrame;
		totalStealTime = 0;
		totalStealCount = 0;
		int STOP_DEPTH = (int)((float)(originalFrame.gridLength * originalFrame.gridLength) * (6.f / 8.f));
		printf("STOP_DEPTH: %d\n", STOP_DEPTH);

		// Solve sudoku
		#pragma omp parallel shared(finalFrame, totalStealCount, totalStealTime) firstprivate(NUM_PROCESSORS) num_threads(NUM_PROCESSORS)
		{
			// Local variables
			int threadId = omp_get_thread_num();
			solType localIsSolved = none;
			sharedLocalVals[threadId].vals.isSolved = false;
			sharedLocalVals[threadId].vals.needMoreGrids = false;
			sharedLocalVals[threadId].vals.solveTimer = Timer();
			sharedLocalVals[threadId].vals.STOP_DEPTH = STOP_DEPTH;
			bool localNeedMoreGrids = false;
			bool firstRun = true;

			if (threadId == 0) {
				// Populate grids initially
				int numPopulated = populateGridsInit(originalFrame, 0, NUM_PROCESSORS);
				// sudokuFrames[0].val = originalFrame;
				if (printInfo) printf("Each thread got their first grid...\n");
				if (printInfo) printf("Number of threads: %d\n", omp_get_num_threads());
				if (printInfo) printf("Took %f.4 s to populate grids initially\n", sharedLocalVals[threadId].vals.solveTimer.elapsed());
				// if (printInfo) sudokuFrames[0].val.displayFrame();
				assert(omp_get_num_threads() == NUM_PROCESSORS);
				assert(numPopulated = NUM_PROCESSORS);
			}

			#pragma omp barrier
			while (!sharedLocalVals[threadId].vals.isSolved) {

				if (!firstRun) {
					if (localNeedMoreGrids == false) {
						localNeedMoreGrids = true;
						#pragma omp atomic write
						sharedLocalVals[(threadId + 1) % NUM_PROCESSORS].vals.needMoreGrids = true;
						// if (printInfo) printf("%d needs more grids!\n", threadId);
					} else if (localNeedMoreGrids == true && sharedLocalVals[(threadId + 1) % NUM_PROCESSORS].vals.needMoreGrids == false) {
						// Our grid has been populated
						// printf("%d got populated!\n", threadId);
						localNeedMoreGrids = false;
					}
				} else {
					firstRun = false;
				}

				// if (threadId == NUM_PROCESSORS - 1) { printf("needMoreGrids val for %d is %d\n", threadId, sharedLocalVals[threadId].vals.needMoreGrids); }

				if (!localNeedMoreGrids) {
					localIsSolved = parallelSolve(sharedLocalVals[threadId].vals.grid, threadId);
				}

				if (localIsSolved == local_solution) {
					if (printInfo) printf("Solved frame in %.6fs:\n", sharedLocalVals[threadId].vals.solveTimer.elapsed());
					#pragma omp critical
					{
						finalFrame = std::move(sharedLocalVals[threadId].vals.grid);
					}
					// Set all "isSolved" to true
					for (int i = 0; i < NUM_PROCESSORS; i++) {
						#pragma omp atomic write
						sharedLocalVals[i].vals.isSolved = true;
					}
				}
			}
		}

		if (printInfo) printf("Total steal time: %.4fs\n", totalStealTime);
		if (printInfo) printf("Total steal count: %d\n", totalStealCount);
		return finalFrame;
	}
	
	/**
	  *	@desc Displays the sudoku frame by calling the displayFrame() func of the
	  *	SudokuFrame object.
	  *	@param none
	  *	@return none
	*/
	void displayFrame(){
		frame.displayFrame();
	}
	
public:

	/**
	  *	@desc The constructor initialises the recursiveCount variable and also calls
	  *	the solve() function which solves the puzzle. It also displays the frames
	  *	before and after the solving.
	  *	@param none
	*/
	SudokuSolver(std::string outputFile = "outputs/output.txt"){
		frame = SudokuFrame(outputFile);
		frame.inputGrid();

		cout<<"\nCalculating possibilities...\n";
		cout<<"Backtracking across puzzle....\n";
		cout<<"Validating cells and values...\n\n";

		auto finalFrame = solve(frame, true);
		cout<<"QED. Your puzzle has been solved!\n\n";
		finalFrame.displayFrame();
		finalFrame.outputGrid();

		cout<<"\n\n";
	}

	// ~SudokuSolver() {
	// 	frame
	// }

	void printAvgTime(int iters) {
		printf("\nCalculating Avg Solve Time (%d iterations)\n", iters);
		double summedTime = 0.0;
		float summedStealTime = 0.0;
		int summedStealCount = 0;
		for (int i = 0; i < iters; i++) {
			Timer oneSolve;
			solve(frame, false);
			double oneSolveTime = oneSolve.elapsed();
			printf(" Iter %d Solve Time: %.6fs\n", i, oneSolveTime); 
			printf(" Iter %d Steal Time: %.6fs\n", i, totalStealTime); 
			printf(" Iter %d Steal Count: %d\n", i, totalStealCount); 
			summedTime += oneSolveTime;
			summedStealTime += totalStealTime;
			summedStealCount += totalStealCount;
		}
		double avgTime = summedTime / iters;
		float avgStealTime = summedStealTime / iters;
		float avgStealCount = (float)summedStealCount / iters;
		printf("\nAverage Solve Time (%d iterations): %.6fs\n", 
			iters, avgTime);
		printf("\nAverage Steal Time (%d iterations): %.6fs\n", 
			iters, avgStealTime);
		printf("\nAverage Steal Count (%d iterations): %.6f\n", 
			iters, avgStealCount);
	}
};

int main(int argc, char *argv[]){
	std::string outputFile = "outputs/output.txt";

	if (argc > 1) {
		outputFile = argv[1];
	}

	cout << outputFile << std::endl;

	// perform one solving, determine solve time
	Timer totalSolveTimer;
	SudokuSolver sudSol(outputFile);
	printf("Total time: %.6fs\n", totalSolveTimer.elapsed());
	
	// calculate & print average solve time
	// sudSol.printAvgTime(ITERATIONS_FOR_AVG);
	return 0;
}
