#include<iostream>
#include<fstream>
#include<cmath>
#include<stack>
#include<vector>
#include<unordered_set>
#include<omp.h>
#include<cassert>
#include "./timing.h"
#define VALMASK 0x7f
#define EDITMASK 0x80 // cell is editable if this bit is NOT SET
#define  printToggle(...) (0)
using namespace std;

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
	std::vector<std::vector<std::vector<char>>> possibilitiesFrame;
	// note: the editable frame has been wrapped into each cell's MSB
	// note: using chars limits the row/col range to <= 127, but that shouldn't be an issue
	char startRow = 0;	// This is how far back the backtracking should go before taking new board from the stack
	char startCol = 0;	// ......................................................................................
	char endRow = 0;	// This is the deepest level that possibilities values were copied from another grid,
	char endCol = 0;	// after this value, the grid should calculate its own possibilities; before or equal to this value, it should not
	char row = 0;
	char col = 0;
	char gridLength = 0;
	char depth = 0;
	bool isDone = false;

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
        endRow = row;
        endCol = col;
		sudokuFrame.resize(gridLength);
		possibilitiesFrame.resize(gridLength);
		
		for (int row = 0; row < gridLength; row++) {
			sudokuFrame[row].resize(gridLength);
			possibilitiesFrame[row].resize(gridLength);
			for (int col = 0; col < gridLength; col++) {
				sudokuFrame[row][col] = oldFrame.sudokuFrame[row][col];
				possibilitiesFrame[row][col] = std::vector<char>(oldFrame.possibilitiesFrame[row][col]);
			}
		}
        setDepth();
	}

	~SudokuFrame() {
		printToggle("Calling delete constructor...\n");
	}

    void setDepth() {
        depth = row * gridLength + col;
    }

    int getDepth() {
        return row * gridLength + col;
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
	  *	@desc Sets cell to not editable.
	  *	@param row (int) row of the required cell
 	  *	@param col (int) col of the required cell
	  *	@return N/A
	*/
	void setNotEditable(int row, int col){
		sudokuFrame[row][col] = sudokuFrame[row][col] | EDITMASK;
		return;
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

	/**	
	  *	@desc Returns the value of the cell at the specified row and col.
	  *	@param row (int) row of the specified cell
	  *	@param col (int) col of the specified cell
	  *	@return (int) cellValue value at the specified cell
	*/
	void setCellValue(int row, int col, char cellValue){
		if(isEditable(row,col)){
			sudokuFrame[row][col] = (cellValue & VALMASK);
		}
		return;
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
		while ((row > startRow || (row == startRow && col >= startCol)) && !isEditable(row, col)) {
			decrementRowCol();
		}

		// Update new endrow
		if ((gridLength * row + col) < (gridLength * endRow + endCol)) {
			endRow = row;
			endCol = col;
		}
		return row > startRow || col >= startCol;
	}
	
	bool updateRowCol() {
		col = (col + 1) % gridLength;
		// row = row + (1 ? col == 0 : 0);
		row = row + (col == 0 ? 1 : 0);
		depth++;

		return row < gridLength;
	}

	inline bool decrementRowCol() {
		col = (col + gridLength - 1) % gridLength;
		// row = row - (1 ? col == (gridLength - 1) : 0);
		row = row - (col == (gridLength - 1) ? 1 : 0);
		depth--;
		return row >= 0;
	}

	void inputGrid() {
		int cinValue;
		std::cin >> cinValue;
		gridLength = (char)cinValue;
		sudokuFrame.resize(gridLength);
		possibilitiesFrame.resize(gridLength);
		for (char row = 0; row < gridLength; row++) {
			sudokuFrame[row].resize(gridLength);
			possibilitiesFrame[row].resize(gridLength);
			for (char col = 0; col < gridLength; col++) {
				possibilitiesFrame[row][col] = std::vector<char>();
				int cinValue;
				std::cin >> cinValue;
				if (cinValue != 0) {
					sudokuFrame[row][col] = (char)cinValue + EDITMASK;
				}
				else {
					sudokuFrame[row][col] = 0;
				}
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
		cout << "\n";
	}
	
};