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
#include "../timing.h"
using namespace std;

/**	
  *	This class provides a data structure which can hold and manipulate the values in a sudoku puzzle.
  *	In this file, we shall call this data structure the 'Sudoku Frame'.
*/
class SudokuFrame{
	
	std::string outputFile; // defaults to "outputs/output.txt";
	public:int gridLength = 0;
	int** sudokuFrame; //This pointer will hold all the values in the matrix.
	bool** editableFrame; //This pointer will tell us all the values which are editable.

	/**	
	  *	@desc This constructor calls the menu() func to provide the menu.
	  *	@param none
	  *	@return none
	*/
	public:SudokuFrame(std::string outputFileV = "outputs/output.txt"){
		outputFile = outputFileV;
	}

	~SudokuFrame() {
		if (gridLength == 0) {
			return;
		}

		for (int row = 0; row < gridLength; row++) {
			delete[] editableFrame[row];
			delete[] sudokuFrame[row];
		}
		
		delete[] editableFrame;
		delete[] sudokuFrame;
	}

	public:void inputGrid() {
		std::cin >> gridLength;
		sudokuFrame = new int*[gridLength];
		editableFrame = new bool*[gridLength];

		for (int row = 0; row < gridLength; row++) {
			sudokuFrame[row] = new int[gridLength];
			editableFrame[row] = new bool[gridLength];
			for (int col = 0; col < gridLength; col++) {
				std::cin >> sudokuFrame[row][col];
				editableFrame[row][col] = true ? (sudokuFrame[row][col] == 0) : false;
			}
		}
		printf("Input grid successful\n");
	}

	public:void outputGrid() {
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
	public:void setCellValue(int row, int col, int num){
		if(editableFrame[row][col]) sudokuFrame[row][col]=num;
	}
	
	/**	
	  *	@desc Returns the value of the cell at the specified row and col.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return (int) cellValue value at the specified cell
	*/
	public:int getCellValue(int row, int col){
		int cellValue=sudokuFrame[row][col];
		return cellValue;
	}
	
	/**
	  *	@desc Returns 0/1 depending on editableFrame values.
	  *	@param row (int) row of the required cell
 	  *	@param col (int) col of the required cell
	  *	@return (int) 1 if editable; 0 if not
	*/
	public:int isEditable(int row, int col){
		return editableFrame[row][col];
	}

	/**
	  *	@desc sets editableFrame value to 0 (lock value).
	  *	@param row (int) row of the required cell
 	  *	@param col (int) col of the required cell
	  *	@return none
	*/
	public:void setNotEditable(int row, int col){
		editableFrame[row][col] = false;
	}

	/**
	  *	@desc Clears frame of all values, other than the question values, from
	  *	the specified cell to the last cell.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	*/
	public:void clearFrameFrom(int row, int col){
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
	
	struct node{
		int value;
		struct node* next;
	}; //The struct for the node
	
	typedef struct node* Node;

	Node head; //The head node
	Node pos; //A node iterator variable
	

	/**	
	  *	@desc This constructor initialises the head (or sentinel) node.
	  *	@param none
	*/
	public:Possibilities(){
		head=new struct node;
		head->next=NULL;
	}
	
	/**	
	  *	@desc This destructor destroys the linked list once the object
 	  *	has finished its lifespan. Calls the destroy() function.
	*/
	public:~Possibilities(){
		destroy();
	}
	
	/**	
	  *	@desc This functions takes in a number and adds it as a node in
	  *	the linked list.
	  *	@param number (int) the number which we want to append
	  *	@return none
	*/
	public:void append(int number){
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
	public:int operator[](int index){
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
	public:void print(){
		pos=head->next;
		while (pos != NULL) {
			cout<<pos->value<<",";
			pos=pos->next;
		}
		cout<<"\n";
	}

	/**
	  *	@desc Returns the length of the linked list.
	  *	@param none
	  *	@return (int) the length of the linked list.
	*/
	public:int length(){
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
	public:void copy(Possibilities possibilities){ // Need to make this clear the old list if exists
		int oldLength=possibilities.length();
		int iter=0;
		
		pos=head;
		pos->next = NULL; // todo: consider lack of free-ing
		for(iter=0; iter<oldLength; iter++){
			Node temp=new struct node;

			temp->next=NULL;
			temp->value=possibilities[iter];

			pos->next=temp;
			pos=pos->next;
		}
	}
	
	/**
	  *	@desc This function searches the possibilties for a certain int value.
	  *	@param int (value) the search value
	  *	@return bool (output) true if value found, false otherwise
	*/
	public:bool searchFor(int value){
		pos = head;
		while(pos != NULL){
			if (pos->value == value) {
				return true;
			}
			pos=pos->next;
		}
		return false;
	}

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

};


/**
  *	Takes in the SudokuFrame object and solves the Sudoku Puzzle.
*/
class SudokuSolver{
	
	int recursiveCount; //Stats variable
	SudokuFrame frame; //The frame object
	Possibilities** gridPoss;
	
	/**
	  *	@desc The constructor initialises the recursiveCount variable and also calls
	  *	the solve() function which solves the puzzle. It also displays the frames
	  *	before and after the solving.
	  *	@param none
	*/
	public:SudokuSolver(std::string outputFile = "outputs/output.txt"){
		frame = SudokuFrame(outputFile);
		frame.inputGrid();
		recursiveCount=0;

		cout<<"\nCalculating possibilities...\n"; // make this actually true
		patternSolve();
		cout<<"Backtracking across puzzle....\n";
		cout<<"Validating cells and values...\n\n";
		
		solve();
		cout<<"QED. Your puzzle has been solved!\n\n";
		displayFrame();
		frame.outputGrid();

		cout<<"\n\n";
	}

	/**
	  *	@desc Checks if the value in the specified cell is valid or not.
	  *	@param row (int) row of the required value
	  *	@param col (int) col of the required value
	  *	@param currentValue (int) the required value
	  *	@return (bool) whether the value is valid or not in the sudoku frame
	*/
	private:bool cellValueValid(int row, int col, int currentValue){
		int rowIter, colIter;

		//Checking if value exists in same column
		for(rowIter=0; rowIter<frame.gridLength; rowIter++){
			if(rowIter!=row){
				int comparingValue=frame.getCellValue(rowIter,col);
				if(comparingValue==currentValue) return false;
			}
		}

		//Checking if value exists in same row
		for(colIter=0; colIter<frame.gridLength; colIter++){
			if(colIter!=col){
				int comparingValue=frame.getCellValue(row,colIter);
				if(comparingValue==currentValue) return false;
			}
		}

		//Checking if value exists in the same 3x3 square block
		if(ThreeByThreeGridValid(row,col,currentValue)==false) return false;

		return true;
	}
	
	/**
	  *	@desc Checks if the same value is also present in the same 3x3 grid block.
	  *	@param row (int) row of the required cell
	  *	@param col (int) col of the required cell
	  *	@param currentValue (int) required value
	  *	@return (bool) whether the value is present or not
	*/
	private:bool ThreeByThreeGridValid(int row, int col, int currentValue){
		int rowStart=(row/(int)(sqrt(frame.gridLength)))*(int)(sqrt(frame.gridLength));
		int rowEnd=rowStart+((int)(sqrt(frame.gridLength)) - 1);
		
		int colStart=(col/(int)(sqrt(frame.gridLength)))*(int)(sqrt(frame.gridLength));
		int colEnd=colStart+((int)(sqrt(frame.gridLength)) - 1);

		int rowIter, colIter;

		for(rowIter=rowStart; rowIter<=rowEnd; rowIter++){
			for(colIter=colStart; colIter<=colEnd; colIter++){
				if(frame.getCellValue(rowIter,colIter)==currentValue) return false;
			}
		}

		return true;	
	}
	
	/**
	  *	@desc Gets the possible values and assigns them to the possibilities list.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return (Possibilities) Possibilities object containing all the possible values.
	*/
	private:Possibilities getCellPossibilities(int row, int col){
		int iter=0;

		Possibilities possibilities;

		// if a frame isn't editable, there are no "possibilities"
		if(!frame.isEditable(row,col)){
			return possibilities;
		}

		for(iter=1; iter<=frame.gridLength; iter++){
			if(cellValueValid(row,col,iter)==true)
				possibilities.append(iter);
		}

		return possibilities;
	}
	
	/**	
	  *	@desc The recursive function which does all the work, this iterates over the
	  *	possible values for the specified cell one-by-one. Once a value has been filled, it
	  *	goes to the next cell. Here, the same thing happens. If none of the possible values
	  *	work out, then the function backtracks to the previous cell.
  	  *
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return (int) whether the value put in the cell made it a SUCCESS or NOT
	*/
	private:int singleCellSolve(int row, int col){
		
		statsIncrement(); //This is used to see how many times the func is called.

		if(frame.isEditable(row,col)==true){

			Possibilities possibilities;
			possibilities.copy(getCellPossibilities(row,col));

			int posLength=possibilities.length();
			int posIter=0, newRow=row, newCol=col;

			for(posIter=0; posIter<posLength; posIter++){ //We iter over the possible values
				int possibility=possibilities[posIter];
				frame.setCellValue(row,col,possibility);
				
				//We now increment the col/row values for the next recursion
				if(col<(frame.gridLength - 1)) newCol=col+1; 
				else if(col==(frame.gridLength - 1)){
					if(row==(frame.gridLength - 1)) return 1; //this means success
					newRow=row+1;
					newCol=0;
				}

				// Recursive call is here
				if(singleCellSolve(newRow,newCol)==0){ //If wrong, clear frame and start over
					frame.clearFrameFrom(newRow,newCol);
				}
				else return 1;
				//Recursion block ends here

			} //Loop ends here

			return 0;
			
		} //The isEditable() if block ends here.
		else{

			int newRow=row, newCol=col;
			
			//Same incrementing of the col/row values
			if(col<(frame.gridLength - 1)) newCol=col+1;
			else if(col==(frame.gridLength - 1)){
				if(row==(frame.gridLength - 1)) return 1;
				newRow=row+1;
				newCol=0;
			}

			return singleCellSolve(newRow,newCol);

		} //The else block ends here

	}

	/**
	  *	@desc Calls the singleCellSolve() func and prints a success/fail mesg.
	  *	@param none
	  *	@return none
	*/
	private:void solve(){
		int success=singleCellSolve(0,0);
	}

	/**
	  *	@desc update global possibilities array.
	  *	@param none
	  *	@return none
	*/
	private:void updatePoss(){
		int gridLen = frame.gridLength;
		for (int row = 0; row < gridLen; row++) {
			for (int col = 0; col < gridLen; col++) {
				gridPoss[row][col].copy(getCellPossibilities(row, col));
			}
		}
	}

	/**
	  *	@desc Uses patterns in rows/cols/subgrids to eliminate possibilities.
	  *	@param none
	  *	@return none
	*/
	private:void patternSolve(){

		//initialize global possibilities / useful vars
		
		int gridLen = frame.gridLength;
		int subGridLen = (int)(sqrt(frame.gridLength));
		gridPoss = new Possibilities*[gridLen];		
		
		// iterate for each cell
		for (int row = 0; row < gridLen; row++) {
			gridPoss[row] = new Possibilities[gridLen];
			for (int col = 0; col < gridLen; col++) {
				gridPoss[row][col].copy(getCellPossibilities(row, col));
			}
		}

		// the main loop begins
		bool unchanged = false;
		while(!unchanged){
			unchanged = true;

			// Apply single-elimination

			for (int row = 0; row < gridLen; row++) {
				for (int col = 0; col < gridLen; col++) {
					if(gridPoss[row][col].length() == 1){
						unchanged = false;
						frame.setCellValue(row,col,gridPoss[row][col][0]);
						frame.setNotEditable(row, col);
						gridPoss[row][col].copy(getCellPossibilities(row, col));
					}
				}
			}

			updatePoss();

			// Apply lone rangers (LRs)

			// init columns arrays and subGrid arrays
			// index1 = colNum / subGrid
			// index2 = 0 to (gridLen-1) value (add 1 for cell value)
			// value = row / subGrid of LR, or NA (-2 or -1)
			int **checkCol = new int*[gridLen];
			int **checkSub = new int*[gridLen];
			for (int i = 0; i < gridLen; i++) {
				checkCol[i] = new int[gridLen];
				checkSub[i] = new int[gridLen];
				for (int j = 0; j < gridLen; j++) {
					checkCol[i][j] = -2;
					checkSub[i][j] = -2;
				}
			}
			
			// check by rows first
			// index = 0 to (gridLen-1) value (add 1 for cell value)
			// value = column of LR, or NA
			int *checkRow = new int[gridLen]; 
			 
			for (int row = 0; row < gridLen; row++) {
				
				// initialize array to zeroes
				for (int i = 0; i < gridLen; i++){
					checkRow[i] = -2;
				}

				// iterate over columns
				for (int col = 0; col < gridLen; col++) {

					int subGridInd = ((row / subGridLen) * subGridLen) 
						+ (col / subGridLen);

					// go through each cell to count possibilities.
					for (int i = 0; i < gridPoss[row][col].length(); i++) {
						int possVal = gridPoss[row][col][i] - 1; 

						// update row array
						if (checkRow[possVal] == -2) {
							checkRow[possVal] = col;
						}
						else { 
							checkRow[possVal] = -1;
						}

						// save to column array to check later
						if (checkCol[col][possVal] == -2) {
							checkCol[col][possVal] = row;
						}
						else { 
							checkCol[col][possVal] = -1;
						}

						// save to subGrid array to check later
						if (checkSub[subGridInd][possVal] == -2) {
							checkSub[subGridInd][possVal] = (row % subGridLen) 
								* subGridLen + (col % subGridLen);
						}
						else { 
							checkSub[subGridInd][possVal] = -1;
						}
					}
				}

				// if no duplicates, it's altering time
				for (int i = 0; i < gridLen; i++){ // i = possibility value
					if (checkRow[i] != -2 && checkRow[i] != -1) {
						int col = checkRow[i];
						unchanged = false;
						frame.setCellValue(row, col, i + 1);
						frame.setNotEditable(row, col);
					}
					
				}	
			}

			// check columns
			for (int col = 0; col < gridLen; col++) {
				
				// if no duplicates, it's altering time
				for (int i = 0; i < gridLen; i++){ // i = possibility value
					if (checkCol[col][i] != -2 && checkCol[col][i] != -1) {
						int row = checkCol[col][i];
						unchanged = false;
						frame.setCellValue(row, col, i + 1);
						frame.setNotEditable(row, col);
					}
				}
			}

			// check subGrids
			for (int subGrid = 0; subGrid < gridLen; subGrid++) {
				// if no duplicates, it's altering time
				for (int i = 0; i < gridLen; i++){ // i = possibility value
					if (checkSub[subGrid][i] != -2 && checkSub[subGrid][i] != -1) {
						// calculate true row/col 
						int rawVal = checkSub[subGrid][i];
						int row = rawVal / subGridLen + 
							(subGrid / subGridLen) * subGridLen;
						int col = rawVal % subGridLen +
							(subGrid % subGridLen) * subGridLen;
						unchanged = false;
						frame.setCellValue(row, col, i + 1);
						frame.setNotEditable(row, col);
					}
				}
			}
			
			// delete arrays
			delete[] checkRow;
			for (int i = 0; i < gridLen; i++) {
				delete[] checkCol[i];
				delete[] checkSub[i];
			}
			delete[] checkCol;
			delete[] checkSub;

			// restart loop if changed
			if (!unchanged){
				updatePoss();
				continue;
			}

		}
		
		// a check for if the patterns have already solved the puzzle 
		bool isSolved = true;
		
		for (int row = 0; row < gridLen && isSolved; row++) {
			for (int col = 0; col < gridLen && isSolved; col++) {
				if (frame.getCellValue(row, col) == 0){
					isSolved = false;
				}
			}
		}
		
		if (isSolved) { 
			cout << "Solved after pattern application!\n";
		}

	}

	/**
	  *	@desc Displays the sudoku frame by calling the displayFrame() func of the
	  *	SudokuFrame object.
	  *	@param none
	  *	@return none
	*/
	private:void displayFrame(){
		frame.displayFrame();
	}
	
	/**
	  *	@desc This increments the count variable to keep track of the recursions done.
	  *	@param none
	  *	@return none
	*/
	private:void statsIncrement(){
		recursiveCount++;
	}

	/**
	  *	@desc This displays the number of times recursion has happened.
	  *	@param none
	  *	@return none
	*/
	public:void statsPrint(){
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
	SudokuSolver sudSol(outputFile);
	printf("Total solve time: %.6fs\n", totalSolveTimer.elapsed());
	
	return 0;
}
