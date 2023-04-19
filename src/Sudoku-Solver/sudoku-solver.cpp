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
#include<set>
#include<omp.h>
#include "../timing.h"
// #define  printToggle(...) (printf(__VA_ARGS__))
#define  printToggle(...) (0)
using namespace std;

const int NUM_PROCESSORS = 8;
enum solType { no_solution, local_solution, global_solution, none };
// const int NO_SOLUTION = 0;
// const int LOCAL_SOLUTION = 1;
// const int GLOBAL_SOLUTION = 2;

/**	
  *	This class provides a data structure which can hold and manipulate the values in a sudoku puzzle.
  *	In this file, we shall call this data structure the 'Sudoku Frame'.
*/
class SudokuFrame{
	
private:

	std::string outputFile;

public:

	// int** sudokuFrame; //This pointer will hold all the values in the matrix.
	// bool** editableFrame; //This pointer will tell us all the values which are editable.
	std::vector<std::vector<int>> sudokuFrame; //This pointer will hold all the values in the matrix.
	std::vector<std::vector<bool>> editableFrame; //This pointer will tell us all the values which are editable.
	int startRow = 0;	// This is how far back the backtracking should go before taking new board from the stack
	int startCol = 0;	// ......................................................................................
	int row = 0;
	int col = 0;
	int gridLength = 0;

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
		sudokuFrame.resize(gridLength);
		editableFrame.resize(gridLength);

		for (int row = 0; row < gridLength; row++) {
			sudokuFrame[row].resize(gridLength);
			editableFrame[row].resize(gridLength);
			for (int col = 0; col < gridLength; col++) {
				sudokuFrame[row][col] = oldFrame.sudokuFrame[row][col];
				editableFrame[row][col] = oldFrame.editableFrame[row][col];
			}
		}
	}

	// // Copy constructor
	// SudokuFrame(const SudokuFrame &oldFrame) {
	// 	printToggle("Calling copy constructor...\n");
	// 	gridLength = oldFrame.gridLength;
	// 	startRow = oldFrame.startRow;
	// 	startCol = oldFrame.startCol;
	// 	row = oldFrame.row;
	// 	col = oldFrame.col;
	// 	sudokuFrame = new int*[gridLength];
	// 	editableFrame = new bool*[gridLength];

	// 	for (int row = 0; row < gridLength; row++) {
	// 		sudokuFrame[row] = new int[gridLength];
	// 		editableFrame[row] = new bool[gridLength];
	// 		for (int col = 0; col < gridLength; col++) {
	// 			sudokuFrame[row][col] = oldFrame.sudokuFrame[row][col];
	// 			editableFrame[row][col] = oldFrame.editableFrame[row][col];
	// 		}
	// 	}
	// }

	~SudokuFrame() {
		printToggle("Calling delete constructor...\n");
	}
	// ~SudokuFrame() {
	// 	printToggle("Calling delete constructor...\n");
	// 	if (gridLength == 0) {
	// 		return;
	// 	}

	// 	for (int row = 0; row < gridLength; row++) {
	// 		delete[] editableFrame[row];
	// 		delete[] sudokuFrame[row];
	// 	}
		
	// 	delete[] editableFrame;
	// 	delete[] sudokuFrame;
	// }

	bool findNextRowCol() {
		while (row < gridLength && !editableFrame[row][col]) {
			updateRowCol();
		}

		return row < gridLength;
	}

	bool findPrevRowCol() {
		decrementRowCol();
		while (row >= 0 && !editableFrame[row][col]) {
			decrementRowCol();
		}

		return row >= 0;
	}
	
	bool updateRowCol() {
		col = (col + 1) % gridLength;
		row = row + (1 ? col == 0 : 0);

		return row < gridLength;
	}

	inline bool decrementRowCol() {
		col = (col + gridLength - 1) % gridLength;
		row = row - (1 ? col == (gridLength - 1) : 0);
		return row >= 0;
	}

	inline int getDepth() {
		return row * gridLength + col;
	}

	void inputGrid() {
		std::cin >> gridLength;
		sudokuFrame.resize(gridLength);
		editableFrame.resize(gridLength);

		for (int row = 0; row < gridLength; row++) {
			sudokuFrame[row].resize(gridLength);
			editableFrame[row].resize(gridLength);
			for (int col = 0; col < gridLength; col++) {
				std::cin >> sudokuFrame[row][col];
				editableFrame[row][col] = true ? (sudokuFrame[row][col] == 0) : false;
			}
		}
		printf("Input grid successful\n");
	}
	// void inputGrid() {
	// 	std::cin >> gridLength;
	// 	sudokuFrame = new int*[gridLength];
	// 	editableFrame = new bool*[gridLength];

	// 	for (int row = 0; row < gridLength; row++) {
	// 		sudokuFrame[row] = new int[gridLength];
	// 		editableFrame[row] = new bool[gridLength];
	// 		for (int col = 0; col < gridLength; col++) {
	// 			std::cin >> sudokuFrame[row][col];
	// 			editableFrame[row][col] = true ? (sudokuFrame[row][col] == 0) : false;
	// 		}
	// 	}
	// 	printToggle("Input grid successful\n");
	// }

	void outputGrid() {
		ofstream myfile;
		myfile.open(outputFile.c_str());

		myfile << gridLength << std::endl;
		for (int i = 0; i < gridLength; i++) {
			for (int j = 0; j < gridLength; j++) {
				myfile << sudokuFrame[i][j] << " ";
			}
			myfile << std::endl;
		}

		myfile.close();
	}
	
	/**
	  *	@desc Assigns the passed-in number to the specified row and col.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return none
	*/
	void setCellValue(int row, int col, int num){
		if(editableFrame[row][col]) sudokuFrame[row][col]=num;
	}
	
	/**	
	  *	@desc Returns the value of the cell at the specified row and col.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return (int) cellValue value at the specified cell
	*/
	int getCellValue(int row, int col){
		int cellValue=sudokuFrame[row][col];
		return cellValue;
	}
	
	/**
	  *	@desc Returns 0/1 depending on editableFrame values.
	  *	@param row (int) row of the required cell
 	  *	@param col (int) col of the required cell
	  *	@return (int) 1 if editable; 0 if not
	*/
	int isEditable(int row, int col){
		return editableFrame[row][col];
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
				if(editableFrame[rowIter][colIter]) sudokuFrame[rowIter][colIter]=0;
			}

			jcount++;

		}
	}

	/**
	  *	@desc Displays the values stored in the frame with designs. We also use
	  *	ANSI colors, using escape sequences, to display the frame.
	  *	@param none
	  *	@return none
	*/
	void displayFrame(){

		cout<<"\033[0;36m++=====================================++";
		int rowIter, colIter;

		for(rowIter=0; rowIter<gridLength; rowIter++){
			int cellIter=1;

			cout<<"\n\033[0;36m||";
			for(colIter=0; colIter<gridLength; colIter++){
				if(cellIter==3){
					cout<<"\033[0m "<<sudokuFrame[rowIter][colIter]<<" ";
					cout<<"\033[0;36m||";
					cellIter=1;
				}
				else{
					cout<<"\033[0m "<<sudokuFrame[rowIter][colIter]<<"  ";
					cellIter++;	
				}
			}

			if(rowIter%3!=2) cout<<"\n\033[0;36m++-----------++-----------++-----------++";
			else cout<<"\n\033[0;36m++=====================================++";
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
		int value;
		struct node* next;
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

	/**	print()

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
	bool isSolved = false;
	int needMoreGrids = 0;
	std::stack<SudokuFrame> gridStack;

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
				if (currFrame.sudokuFrame[rowIter][col] == currentValue) return false;
			}
		}

		//Checking if value exists in same row
		for(colIter = 0; colIter < frame.gridLength; colIter++){
			if(colIter != col){
				if (currFrame.sudokuFrame[row][colIter] == currentValue) return false;
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
				if((rowIter != row || colIter != col) && currFrame.sudokuFrame[rowIter][colIter] == currentValue) return false;
			}
		}

		return true;	
	}
	
	// /**
	//   *	@desc Gets the possible values and assigns them to the possibilities list.
	//   *	@param row (int) row of the specified cell
	//   *	@param col (int) col of the specified cell
	//   *	@return (Possibilities) Possibilities object containing all the possible values.
	// */
	// Possibilities getCellPossibilities(int row, int col){
	// 	int iter=0;

	// 	Possibilities possibilities;

	// 	for(iter=1; iter<=frame.gridLength; iter++){
	// 		if(cellValueValid(row,col,iter)==true)
	// 			possibilities.append(iter);
	// 	}

	// 	return possibilities;
	// }

	std::vector<int> getCellPossibilities(SudokuFrame &currFrame){
		int iter = 0;
		int row = currFrame.row;
		int col = currFrame.col;

		std::vector<int> possibilities;
		std::set<int> possibilitiesSet;

		// Add all items to set initially
		for (int i = 1; i <= currFrame.gridLength; i++) {
			possibilitiesSet.insert(i);
		}

		// Removing values in same column
		for(int rowIter = 0; rowIter < currFrame.gridLength; rowIter++){
			if(rowIter != row){
				possibilitiesSet.erase(currFrame.sudokuFrame[rowIter][col]);
			}
		}

		// Remoiving values in same row
		for(int colIter = 0; colIter < frame.gridLength; colIter++){
			if(colIter != col){
				possibilitiesSet.erase(currFrame.sudokuFrame[row][colIter]);
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
					possibilitiesSet.erase(currFrame.sudokuFrame[rowIter][colIter]);
				}
			}
		}

		for (auto elem : possibilitiesSet) {
			possibilities.push_back(elem);
		}

		return possibilities;
	}
	
	// /**	
	//   *	@desc The recursive function which does all the work, this iterates over the
	//   *	possible values for the specified cell one-by-one. Once a value has been filled, it
	//   *	goes to the next cell. Here, the same thing happens. If none of the possible values
	//   *	work out, then the function backtracks to the previous cell.
  	//   *
	//   *	@param row (int) row of the specified cell
	//   *	@param col (int) col of the specified cell
	//   *	@return (int) whether the value put in the cell made it a SUCCESS or NOT
	// */
	// int singleCellSolve(SudokuFrame &currFrame, int row, int col) {
		
	// 	statsIncrement(); //This is used to see how many times the func is called.

	// 	if(frame.isEditable(row,col)==true){

	// 		Possibilities possibilities;
	// 		possibilities.copy(getCellPossibilities(row,col));

	// 		int posLength=possibilities.length();
	// 		int newRow=row, newCol=col;

	// 		for(int posIter=0; posIter<posLength; posIter++){ //We iter over the possible values
	// 			int possibility=possibilities[posIter];
	// 			frame.setCellValue(row,col,possibility);
				
	// 			//We now increment the col/row values for the next recursion
	// 			if(col<(frame.gridLength - 1)) newCol=col+1; 
	// 			else if(col==(frame.gridLength - 1)){
	// 				if(row==(frame.gridLength - 1)) return 1; //this means success
	// 				newRow=row+1;
	// 				newCol=0;
	// 			}

	// 			{

	// 				if(singleCellSolve(newRow,newCol)==0){ //If wrong, clear frame and start over
	// 					frame.clearFrameFrom(newRow,newCol);
	// 				}
	// 				else return 1;

	// 			} //Recursion block ends here

	// 			} //Loop ends here

	// 		return 0;
			
	// 	} //The isEditable() if block ends here.
	// 	else{

	// 		int newRow=row, newCol=col;
			
	// 		//Same incrementing of the col/row values
	// 		if(col<(frame.gridLength - 1)) newCol=col+1;
	// 		else if(col==(frame.gridLength - 1)){
	// 			if(row==(frame.gridLength - 1)) return 1;
	// 			newRow=row+1;
	// 			newCol=0;
	// 		}

	// 		return singleCellSolve(newRow,newCol);

	// 	} //The else block ends here

	// }

	solType parallelSolve(SudokuFrame &currFrame, int threadId) {
		
		if (isSolved) {
			// Another thread has a valid solution
			return global_solution;
		} else if (currFrame.row == currFrame.gridLength && currFrame.col == 0) {
			// We have a valid solution
			// currFrame.displayFrame();
			// auto temp = std::cin.get();
			return local_solution;
		}

		// Try a value in an editable cell
		int row = currFrame.row;
		int col = currFrame.col;
		if(currFrame.editableFrame[row][col]) {

			std::vector<int> possibilities = getCellPossibilities(currFrame);
			// if (threadId == 0) {
			// 	printf("(%d, %d)\n", row, col);
			// }

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
		else{
			// Seek first editable chunk
			currFrame.findNextRowCol();

			return parallelSolve(currFrame, threadId);
		}
	}
	
	// Populates stack by expanding topmost grid in stack in BFS fashion
	bool populateStackBFS(int fillSize) {
		std::vector<int> currPossibilities;
		SudokuFrame grid;
		int count = 0;

		while(gridStack.size() < fillSize && count < fillSize) {
			if (gridStack.size() > 0) {
				grid = SudokuFrame(gridStack.top());
				printToggle("grid frame length: %ld\n", grid.editableFrame.size());
				gridStack.pop();
				printToggle("grid frame length after pop: %ld\n", grid.editableFrame.size());
			} else {
				perror("Grid stack empty");
				return false;
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
					newFrame.startRow = row;
					newFrame.startCol = col;
					newFrame.updateRowCol();
					gridStack.push(newFrame);
					// printf("Gridstack size: %d\n", gridStack.size());
				}
			}
			count++;
		}	

		return true;
	}

	/**
	  *	@desc Calls the singleCellSolve() func and prints a success/fail mesg.
	  *	@param none
	  *	@return none
	*/
	SudokuFrame solve(SudokuFrame &originalFrame) {
		// Create final grid
		SudokuFrame finalFrame;

		// Populate stack initially
		printf("About to push to stack originally...\n");
		gridStack.push(originalFrame);
		printf("Pushed to stack...\n");
		populateStackBFS(NUM_PROCESSORS);
		printf("Populated stack to size: %ld\n", gridStack.size());

		// Solve sudoku
		#pragma omp parallel shared(isSolved, gridStack, needMoreGrids, finalFrame) num_threads(NUM_PROCESSORS)
		{
			// Local variables
			SudokuFrame localFrame;
			solType localIsSolved = none;
			int threadId = omp_get_thread_num();
			bool localNeedMoreGrids = false;

			while (!isSolved) {
				#pragma omp critical
				{
					if (gridStack.size() > 0) {
						localFrame = gridStack.top(); gridStack.pop();
						if (localNeedMoreGrids) {
							localNeedMoreGrids = false;
							needMoreGrids--;
						}
					} else {
						localNeedMoreGrids = true;
						needMoreGrids++;
					}
				}

				// Solve the local frame
				// printf("Solving...\n");
				if (!localNeedMoreGrids) {
					localIsSolved = parallelSolve(localFrame, threadId);
				}

				if (localIsSolved == local_solution) {
					#pragma omp critical
					{
						isSolved = true;
						// printf("Solved frame:\n");
						// localFrame.displayFrame();
						finalFrame = SudokuFrame(localFrame);
					}
				}
			}
		}

		// Delete variables

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

		auto finalFrame = solve(frame);
		cout<<"QED. Your puzzle has been solved!\n\n";
		finalFrame.displayFrame();
		finalFrame.outputGrid();

		cout<<"\n\n";
	}

	// ~SudokuSolver() {
	// 	frame
	// }
	
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

	Timer totalSolveTimer;
	SudokuSolver ss(outputFile);
	printf("Total solve time: %.6fs\n", totalSolveTimer.elapsed());
	return 0;
}
