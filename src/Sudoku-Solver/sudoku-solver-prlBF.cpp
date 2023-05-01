/*
 *=====================
 *   Sudoku Solver
 *=====================
 *
 * Started On - October 2nd, 2018
 * Author - Arjun Aravind
 * Objective - Takes in Sudoku puzzles and outputs the solution.
*/

#include<iostream>
#include<fstream>
#include<cmath>
#include<stack>
#include<vector>
#include<unordered_set>
#include<omp.h>
#include<cassert>
#include "../timing.h"
// #define  printToggle(...) (printf(__VA_ARGS__))
#define  printToggle(...) (0)
#define VALMASK 0x7f
#define EDITMASK 0x80 // cell is editable if this bit is NOT SET
using namespace std;

const int NUM_PROCESSORS = 16;
const int CACHE_LINE_SIZE = 64;
const int ITERATIONS_FOR_AVG = 50;
int STOP_DEPTH = 0;
enum solType { no_solution, local_solution, global_solution, none };

struct paddedBool {
	bool isSolved;
	char padding[CACHE_LINE_SIZE - (sizeof(bool) % CACHE_LINE_SIZE)];
};

struct paddedInt {
	int needMoreGrids;
	char padding[CACHE_LINE_SIZE - (sizeof(int) % CACHE_LINE_SIZE)];
};

/**	
  *	This class provides a data structure which can hold and manipulate the values in a sudoku puzzle.
  *	In this file, we shall call this data structure the 'Sudoku Frame'.
*/
class SudokuFrame{
	
private:

	std::string outputFile;

public:
	//This pointer will hold all the values in the matrix
	std::vector<std::vector<char>> sudokuFrame;
	// note: the editable frame has been wrapped into each cell's MSB
	// note: using chars limits the row/col range to <= 127, but that shouldn't be an issue
	char startRow = 0;	// This is how far back the backtracking should go before taking new board from the stack
	char startCol = 0;	// ......................................................................................
	char row = 0;
	char col = 0;
	char gridLength = 0;
	char depth = 0;

	/**	
	  *	@desc This constructor calls the menu() func to provide the menu.
	  *	@param none
	  *	@return none
	*/
	SudokuFrame(std::string outputFileV = "outputs/output.txt"){
		outputFile = outputFileV;
	}

	// Copy constructor
	SudokuFrame(const SudokuFrame &oldFrame) {
		printToggle("Calling copy constructor...\n");
		gridLength = oldFrame.gridLength;
		startRow = oldFrame.startRow;
		startCol = oldFrame.startCol;
		row = oldFrame.row;
		col = oldFrame.col;
		//editableFrame = oldFrame.editableFrame;
		sudokuFrame.resize(gridLength);
		//editableFrame.resize(gridLength);
		
		for (int row = 0; row < gridLength; row++) {
			sudokuFrame[row].resize(gridLength);
			//editableFrame[row].resize(gridLength);
			for (int col = 0; col < gridLength; col++) {
				sudokuFrame[row][col] = oldFrame.sudokuFrame[row][col];
				//editableFrame[row][col] = oldFrame.editableFrame[row][col];
			}
		}
	}

	~SudokuFrame() {
		printToggle("Calling delete constructor...\n");
	}

	/**
	  *	@desc Returns if cell is editable.
	  *	@param row (int) row of the required cell
 	  *	@param col (int) col of the required cell
	  *	@return 1 if editable; 0 if not
	*/
	bool isEditable(int row, int col){
		return ((sudokuFrame[row][col] & EDITMASK) != EDITMASK);
	}

	/**	
	  *	@desc Returns the value of the cell at the specified row and col.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return (int) cellValue value at the specified cell
	*/
	int getCellValue(int row, int col){
		int cellValue = (int)(sudokuFrame[row][col] & VALMASK);
		return cellValue;
	}

	// Includes current index
	bool findNextRowCol() {
		while (row < gridLength && !(isEditable(row, col))) {
			updateRowCol();
		}

		return row < gridLength;
	}

	// Excludes current index
	bool findPrevRowCol() {
		decrementRowCol();
		while ((row > startRow || col >= startCol) && !isEditable(row, col)) {
			decrementRowCol();
		}

		return row > startRow || col >= startCol;
	}
	
	bool updateRowCol() {
		col = (col + 1) % gridLength;
		row = row + (1 ? col == 0 : 0);
		depth++;

		return row < gridLength;
	}

	inline bool decrementRowCol() {
		col = (col + gridLength - 1) % gridLength;
		row = row - (1 ? col == (gridLength - 1) : 0);
		depth--;
		return row >= 0;
	}

	void inputGrid() {
		int cinValue;
		std::cin >> cinValue;
		gridLength = (char)cinValue;
		sudokuFrame.resize(gridLength);
		//editableFrame.resize(gridLength);
		for (char row = 0; row < gridLength; row++) {
			sudokuFrame[row].resize(gridLength);
			//editableFrame[row].resize(gridLength);
			for (char col = 0; col < gridLength; col++) {
				int cinValue;
				std::cin >> cinValue;
				if (cinValue != 0) {
					sudokuFrame[row][col] = (char)cinValue + EDITMASK;
				}
				else {
					sudokuFrame[row][col] = 0;
				}
				//editableFrame[row][col] = true ? (sudokuFrame[row][col] == 0) : false;
			}
		}
		printf("Input grid successful\n");
	}

	void outputGrid() {
		ofstream myfile;
		myfile.open(outputFile.c_str());

		myfile << gridLength << std::endl;
		for (int i = 0; i < gridLength; i++) {
			for (int j = 0; j < gridLength; j++) {
				myfile << getCellValue(i,j) << " ";
			}
			myfile << std::endl;
		}

		myfile.close();
	}
	

	/**
	  *	@desc Clears frame of all values, other than the question values, from
	  *	the specified cell to the last cell.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	*/
	void clearFrameFrom(int row, int col){
		int jcount=0;
		int rowIter, colIter;

		for(rowIter=row; rowIter<gridLength; rowIter++){
			
			if(jcount==0) colIter=col;
			else colIter=0;

			for(; colIter<gridLength; colIter++){
				if(isEditable(rowIter, colIter)) sudokuFrame[rowIter][colIter]=0;
			}

			jcount++;

		}
	}

	/**
	  *	@desc Displays the values stored in the frame with designs. We also use
	  *	ANSI colors, using escape sequences, to display the frame.
	  * Note: this is hardcoded for 2-digit wide numbers (in decimal)
	  *	@param none
	  *	@return none
	*/
	void displayFrame(){
		
		int gridRoot = (int)(sqrt(gridLength));
		
		// assuming that for each subgrid row:
		// 4 spaces per cell (includes front and back padding)
		// 2 spaces per subgrid delineation
		// 2 spaces (negative) for the end '++'s
		
		int dashWidth = 4 * gridRoot;
		int eqWidth = gridRoot * (dashWidth + 2) - 2;
		
		// generate "++==...==++" thing
		cout << "\033[0;36m++";
		for (int i = 0; i < eqWidth; i++) cout << '=';
		cout << "++";

		int rowIter, colIter;
		for(rowIter=0; rowIter<gridLength; rowIter++){
			int cellIter=1;

			cout<<"\n\033[0;36m||";
			for(colIter=0; colIter<gridLength; colIter++){
				
				// note: this assumes 2-digit wideness
				if ((int)getCellValue(rowIter,colIter) < 10) {
					cout<<"\033[0m "<<(int)getCellValue(rowIter,colIter)<<"  ";
				}
				else {
					cout<<"\033[0m "<<(int)getCellValue(rowIter,colIter)<<" ";
				}

				if(cellIter == gridRoot){
					cout<<"\033[0;36m||";
					cellIter=1;
				}
				else{
					cellIter++;	
				}
			}

			if ((rowIter % gridRoot) != (gridRoot - 1)) {
				cout << "\n\033[0;36m++";
				for (int i = 0; i < gridRoot; i++) {
					for (int j = 0; j < dashWidth; j++) {
						cout << '-';
					}
					cout << "++";
				}
			}
			else {
				// generate "++==...==++" thing
				cout << "\n\033[0;36m++";
				for(int i = 0; i < eqWidth; i++) cout << '=';
				cout << "++";
			}
		}

	}
	
};


/**	
  *	This class provides the programmer a very simple way to iterate through
  *	the possibilities of a specified cell. This object utilises linked lists.
*/
class Possibilities{
	
private:

	struct node{
		struct node* next;
		char value;
	}; //The struct for the node
	
	typedef struct node* Node;

	Node head; //The head node
	Node pos; //A node iterator variable
	
	/**
	  *	@desc Frees all the nodes in the linked list.
	  *	@param none
	  *	@return none
	*/
	private:void destroy(){
		Node temp;
		pos=head;
		while(pos!=NULL){
			temp=pos;
			pos=pos->next;
			delete temp;
		}
	}

public:

	/**	
	  *	@desc This constructor initialises the head (or sentinel) node.
	  *	@param none
	*/
	Possibilities(){
		head=new struct node;
		head->next=NULL;
	}
	
	/**	
	  *	@desc This destructor destroys the linked list once the object
 	  *	has finished its lifespan. Calls the destroy() function.
	*/
	~Possibilities(){
		destroy();
	}
	
	/**	
	  *	@desc This functions takes in a number and adds it as a node in
	  *	the linked list.
	  *	@param number (int) the number which we want to append
	  *	@return none
	*/
	void append(int number){
		Node temp=new struct node;

		temp->value=number;
		temp->next=NULL;

		pos=head;
		while(pos!=NULL){
			if(pos->next==NULL){
				pos->next=temp;
				break;
			}
			pos=pos->next;
		}
	}
	
	/**	
	  *	@desc An operator overload function which overloads the [] operator.
	  *	@param index (int) the index of the required node in the linked list.
	  *	@return (int) the value contained in the specified node.
	*/
	int operator[](int index){
		int count=0;
		pos=head->next;
		
		while(pos!=NULL){
			if(count==index)
				return pos->value;
			pos=pos->next;
			count++;
		}

		return -1;
	}

	/**	
	  *	@desc Prints the values inside all the nodes of the linked list.
	  *	@param none
	  *	@return none
	*/
	void print(){
		pos=head->next;
		while(pos!=NULL){
			cout<<pos->value<<",";
			pos=pos->next;
		}
		cout<<"\b";
	}

	/**
	  *	@desc Returns the length of the linked list.
	  *	@param none
	  *	@return (int) the length of the linked list.
	*/
	int length(){
		pos=head->next;
		int count=0;

		while(pos!=NULL){
			count++;
			pos=pos->next;
		}
		
		return count;
	}
	
	/**
	  *	@desc This function takes in a possibilities object and copies
	  *	the contents into THIS object.
	  *	@param possibilities (Possibilities) the object which is to be copied
	  *	@return none
	*/
	void copy(Possibilities possibilities){ //Need to make this clear the old list if exists
		int oldLength=possibilities.length();
		int iter=0;
		
		pos=head;
		for(iter=0; iter<oldLength; iter++){
			Node temp=new struct node;

			temp->next=NULL;
			temp->value=possibilities[iter];

			pos->next=temp;
			pos=pos->next;
		}
	}
};


/**
  *	Takes in the SudokuFrame object and solves the Sudoku Puzzle.
*/
class SudokuSolver{
	
private:

	int recursiveCount; //Stats variable
	SudokuFrame frame; //The frame object

	// Create shared variables
	// bool isSolved = false;
	paddedBool isSolved[NUM_PROCESSORS];
	paddedInt needMoreGrids[NUM_PROCESSORS];
	// int needMoreGrids = 0;
	std::stack<SudokuFrame> gridStack;
	omp_lock_t stackLock;

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
	
	std::vector<int> getCellPossibilities(SudokuFrame &currFrame){
		int iter = 0;
		int row = currFrame.row;
		int col = currFrame.col;

		std::vector<int> possibilities;
		std::unordered_set<int> possibilitiesSet;

		// Add all items to set initially
		for (int i = 1; i <= currFrame.gridLength; i++) {
			possibilitiesSet.insert(i);
		}

		// Removing values in same column
		for(int rowIter = 0; rowIter < currFrame.gridLength; rowIter++){
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
	
	solType parallelSolve(SudokuFrame &currFrame, int threadId) {
		
		if (isSolved[threadId].isSolved) {
			// Another thread has a valid solution
			return global_solution;
		} else if (currFrame.row >= currFrame.gridLength) {
			// We have a valid solution
			return local_solution;
		}

		// Add grids to stack if previous thread needs; test-and-test-and-set
		if (currFrame.depth < STOP_DEPTH && needMoreGrids[(threadId + NUM_PROCESSORS - 1) % NUM_PROCESSORS].needMoreGrids > 0) {
			omp_set_lock(&stackLock);
			if (needMoreGrids[(threadId + NUM_PROCESSORS - 1) % NUM_PROCESSORS].needMoreGrids > 0) {
				// printf("Adding grids to stack. needMoreGrids = %d\n", needMoreGrids);
				gridStack.push(currFrame);
				auto gridSol = populateStackBFS(needMoreGrids[(threadId + NUM_PROCESSORS - 1) % NUM_PROCESSORS].needMoreGrids + 1);

				auto newFrame = std::move(gridStack.top()); gridStack.pop();

				auto framesAdded = gridStack.size();
				// printf("framesAdded: %d\n", framesAdded);
				for (int i = 0; i < NUM_PROCESSORS - 1; i++) {
					if (framesAdded <= 0) {
						break;
					} else if (needMoreGrids[(threadId + NUM_PROCESSORS - 1 - i) % NUM_PROCESSORS].needMoreGrids > 0) {
						#pragma omp atomic
						needMoreGrids[(threadId + NUM_PROCESSORS - 1 - i) % NUM_PROCESSORS].needMoreGrids -= 1;
						framesAdded--;
					}
				}

				omp_unset_lock(&stackLock);
				if (gridSol == none) {
					return parallelSolve(newFrame, threadId);
				} else {
					// local solution or no solution
					return gridSol;
				}
			} else {
				omp_unset_lock(&stackLock);
			}
		}

		// Try a value in an editable cell
		int row = currFrame.row;
		int col = currFrame.col;
		if(currFrame.isEditable(row,col)) {

			std::vector<int> possibilities = getCellPossibilities(currFrame);

			// Iterate over possible values
			for(int posIter = 0; posIter < possibilities.size(); posIter++) {	
				currFrame.sudokuFrame[row][col] = possibilities[posIter];
				
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
	
	// Populates stack by expanding topmost grid in stack in BFS fashion; if no grids are added, returns solType of grid
	solType populateStackBFS(int fillSize) {
		std::vector<int> currPossibilities;
		SudokuFrame grid;
		// int filledAmount = 0;

		if (gridStack.size() == 0) {
			perror("Grid stack empty");
			return none;
		}

		while(gridStack.size() < fillSize) {
			if (gridStack.size() > 0) {
				grid = std::move(SudokuFrame(gridStack.top()));
				printToggle("grid frame length: %ld\n", grid.sudokuFrame.size());
				gridStack.pop();
				// filledAmount -= 1;
				printToggle("grid frame length after pop: %ld\n", grid.sudokuFrame.size());
			} else {
				// No more grids on the stack; no_solution
				gridStack.push(grid);
				return no_solution;
			}

			printToggle("About to check editable frame...\n");
			// printf("row before: %d, col before: %d\n", grid.row, grid.col);
			if (grid.findNextRowCol()) {
				// printf("row after: %d, col after: %d\n", grid.row, grid.col);
				int row = grid.row;
				int col = grid.col;
				// printf("About to check possibilities...\n");
				currPossibilities = getCellPossibilities(grid);
				// printf("Found %ld possibilities...\n", currPossibilities.size());

				// Push new grids onto stack
				for (auto value : currPossibilities) {
					SudokuFrame newFrame = SudokuFrame(grid);
					newFrame.sudokuFrame[row][col] = value;
					newFrame.updateRowCol();
					newFrame.findNextRowCol();
					newFrame.startRow = newFrame.row; // Set to first index newFrame can edit
					newFrame.startCol = newFrame.col;
					gridStack.push(std::move(newFrame));
					// printf("Gridstack size: %d\n", gridStack.size());
				}

				// filledAmount += currPossibilities.size();
			} else {
				// Found solution board; push back on and return
				gridStack.push(std::move(grid));
				return local_solution;
			}
		}	
		// printf("grid stack size: %d\n", gridStack.size());

		return none;
	}

	/**
	  *	@desc Calls the singleCellSolve() func and prints a success/fail mesg.
	  *	@param none
	  *	@return none
	*/
	SudokuFrame solve(SudokuFrame &originalFrame, bool printInfo) {
		// Create final grid
		SudokuFrame finalFrame;
		Timer solveTimer;
		omp_init_lock(&stackLock);
		STOP_DEPTH = (int)((float)(originalFrame.gridLength * originalFrame.gridLength) * (2.f / 8.f));

		// Populate stack initially
		if (printInfo) printf("About to push to stack originally...\n");
		gridStack.push(originalFrame);
		if (printInfo) printf("Pushed to stack...\n");
		populateStackBFS(NUM_PROCESSORS);
		if (printInfo) printf("Populated stack to size: %ld\n", gridStack.size());

		// Solve sudoku
		#pragma omp parallel shared(isSolved, gridStack, needMoreGrids, finalFrame, stackLock) num_threads(NUM_PROCESSORS)
		{
			// Local variables
			SudokuFrame localFrame;
			solType localIsSolved = none;
			int threadId = omp_get_thread_num();
			isSolved[threadId].isSolved = false;
			needMoreGrids[threadId].needMoreGrids = 0;
			bool localNeedMoreGrids = false;

			if (threadId == 0) {
				if (printInfo) printf("Each thread got their first grid...\n");
				if (printInfo) printf("Number of threads: %d\n", omp_get_num_threads());
				assert(omp_get_num_threads() == NUM_PROCESSORS);
			}

			#pragma omp barrier
			while (!isSolved[threadId].isSolved) {

				if (needMoreGrids[threadId].needMoreGrids == 0) {
					omp_set_lock(&stackLock);
					// printf("Set lock at address: %x\n", &stackLock);
					if (gridStack.size() > 0) {
						localFrame = std::move(gridStack.top()); gridStack.pop();
						localNeedMoreGrids = false;
					} 
					else if (localNeedMoreGrids == false) {
						localNeedMoreGrids = true;
						#pragma omp atomic
						needMoreGrids[threadId].needMoreGrids++;
					}
					omp_unset_lock(&stackLock);
				} else if (localNeedMoreGrids == false) {
					localNeedMoreGrids = true;
					#pragma omp atomic
					needMoreGrids[threadId].needMoreGrids++;
				}

				if (!localNeedMoreGrids) {
					// printf("Solving...\n");
					// localFrame.displayFrame();
					// printf("\n");
					localIsSolved = parallelSolve(localFrame, threadId);
					// localFrame.displayFrame();
					// printf("Finished Solving... got code: %d \n", localIsSolved);
				}

				if (localIsSolved == local_solution) {
					#pragma omp critical
					{
						if (printInfo) printf("Solved frame in %.6fs:\n", solveTimer.elapsed());
						// localFrame.displayFrame();
						finalFrame = std::move(localFrame);
						// finalFrame = SudokuFrame(localFrame);
					}
					// Set all "isSolved" to true
					for (int i = 0; i < NUM_PROCESSORS; i++) {
						#pragma omp atomic write
						isSolved[i].isSolved = true;
					}
				}
			}
		}

		// Delete variables
		omp_destroy_lock(&stackLock);

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
	
	/**
	  *	@desc This increments the count variable to keep track of the recursions done.
	  *	@param none
	  *	@return none
	*/
	void statsIncrement(){
		recursiveCount++;
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
		recursiveCount=0;

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
		for (int i = 0; i < iters; i++) {
			Timer oneSolve;
			solve(frame, false);
			double oneSolveTime = oneSolve.elapsed();
			printf(" Iter %d Solve Time: %.6fs\n", i, oneSolveTime); 
			summedTime += oneSolveTime;
		}
		double avgTime = summedTime / iters;
		printf("\nAverage Solve Time (%d iterations): %.6fs\n", 
			iters, avgTime);
	}
	
	/**
	  *	@desc This displays the number of times recursion has happened.
	  *	@param none
	  *	@return none
	*/
	void statsPrint(){
		cout<<"\nThe singleCellSolve() function was recursively called "<<recursiveCount<<" times.\n";
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
