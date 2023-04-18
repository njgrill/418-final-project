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
	
private:

	std::string outputFile;

public:

	int** sudokuFrame; //This pointer will hold all the values in the matrix.
	bool** editableFrame; //This pointer will tell us all the values which are editable.
	int gridLength = 0;

	/**	
	  *	@desc This constructor calls the menu() func to provide the menu.
	  *	@param none
	  *	@return none
	*/
	SudokuFrame(std::string outputFileV = "outputs/output.txt"){
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

	void inputGrid() {
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

	/**
	  *	@desc Checks if the value in the specified cell is valid or not.
	  *	@param row (int) row of the required value
	  *	@param col (int) col of the required value
	  *	@param currentValue (int) the required value
	  *	@return (bool) whether the value is valid or not in the sudoku frame
	*/
	bool cellValueValid(int row, int col, int currentValue){
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
	bool ThreeByThreeGridValid(int row, int col, int currentValue){
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
	Possibilities getCellPossibilities(int row, int col){
		int iter=0;

		Possibilities possibilities;

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
	int singleCellSolve(int row, int col){
		
		statsIncrement(); //This is used to see how many times the func is called.

		if(frame.isEditable(row,col)==true){

			Possibilities possibilities;
			possibilities.copy(getCellPossibilities(row,col));

			int posLength=possibilities.length();
			int newRow=row, newCol=col;

			for(int posIter=0; posIter<posLength; posIter++){ //We iter over the possible values
				int possibility=possibilities[posIter];
				frame.setCellValue(row,col,possibility);
				
				//We now increment the col/row values for the next recursion
				if(col<(frame.gridLength - 1)) newCol=col+1; 
				else if(col==(frame.gridLength - 1)){
					if(row==(frame.gridLength - 1)) return 1; //this means success
					newRow=row+1;
					newCol=0;
				}

				{

					if(singleCellSolve(newRow,newCol)==0){ //If wrong, clear frame and start over
						frame.clearFrameFrom(newRow,newCol);
					}
					else return 1;

				} //Recursion block ends here

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
	void solve(){
		int success=singleCellSolve(0,0);
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
		
		solve();
		cout<<"QED. Your puzzle has been solved!\n\n";
		displayFrame();
		frame.outputGrid();

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
