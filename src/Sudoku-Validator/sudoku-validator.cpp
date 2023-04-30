/*
 *========================
 *    Sudoku Validator
 *========================
 *
 * Started On: October 18th, 2018
 * Author: Arjun Aravind
 * Objective: To verify whether a completed Sudoku puzzle is valid or not.
*/

#include<iostream>
#include<fstream>
#include<cmath>
#include "../SudokuFrame.cpp"
using namespace std;

class SudokuValidator{

	SudokuFrame frame;
	
	/**
	  *	@desc Calls the function which validates the values in the Sudoku frame.
	  *	@param none
	 */
	public:SudokuValidator(){
		validateFrame();		
	}

	/**
	  *	@desc Calls functions which validates the rows, columns and 3x3 grids.
	  *	@param none
	  *	@return none
	*/
	private:void validateFrame(){
		if(valuesAreValid()){
			if(rowsAreValid()){
				if(columnsAreValid()){
					if(ThreeByThreeSquaresAreValid()){
						cout<<"\nYour puzzle is valid!\n\n";
					}
					else cout<<"Your puzzle squares are invalid!\n";
				}
				else cout<<"Your puzzle columns are invalid!\n";
			}
			else cout<<"Your rows are invalid!\n";
		}
		else cout<<"Your puzzle values are invalid!\n";
	}
	

	/**
	  *	@desc Checks if all values in the frame are between the ranges of 1-frame.gridLength.
	  *	@param none
	  *	@return (bool) Whether all the values are valid or not.
	*/
	private:bool valuesAreValid(){		
		int rowIter, colIter;

		for(rowIter=0; rowIter<frame.gridLength; rowIter++){
			for(colIter=0; colIter<frame.gridLength; colIter++){
				int cellValue=frame.getCellValue(rowIter,colIter);
				if(!(cellValue>=1 && cellValue<=frame.gridLength)) return false;
			}
		}

		return true;
	}

	/**
	  *	@desc Checks if the rows are valid in the frame.
	  *	@param none	
	  *	@return (bool) Whether the rows are valid or not.
	*/
	private:bool rowsAreValid(){
		
		int rowIter, colIter, valIter;

		for(rowIter=0; rowIter<frame.gridLength; rowIter++){ //Iterating over rows

			for(valIter=1; valIter<=frame.gridLength; valIter++){ //Iter over 1-frame.gridLength

				if(rowContains(rowIter,valIter)==false) {
					printf("Row contains (%d, %d) is false\n", rowIter, valIter);
					return false;
				}
			} //Value iteration loop ends here

		} //Row iteration loop ends here

		return true;
	}

	/**
	  *	@desc Checks if the columns in the frame are valid.
	  *	@param none
	  *	@return (bool) Whether the columns are valid or not.
	*/
	private:bool columnsAreValid(){
		
		int rowIter, colIter, valIter;
		
		for(colIter=0; colIter<frame.gridLength; colIter++){ //Iterating over cols
		
			for(valIter=1; valIter<=frame.gridLength; valIter++){ //Iter over 1-frame.gridLength
				
				if(columnContains(colIter,valIter)==false)
					return false;

			} //Value iteration loops ends

		} //Col iteration loop ends here
		
		return true;
	}
	
	/**
	  *	@desc Checks if the 3x3 grids in the frame are valid.
	  *	@param none
	  *	@return (bool) Whether the 3x3 grids are valid or not.
	*/
	private:bool ThreeByThreeSquaresAreValid(){
		
		int squareIter, valIter;

		for(squareIter=0; squareIter<frame.gridLength; squareIter++){ //Iter over squares
			
			for(valIter=1; valIter<=frame.gridLength; valIter++){ //Iter over 1-frame.gridLength

				if(squareContains(squareIter,valIter)==false)
					return false;

			} //Value iteration loop ends here

		} //Square iteration loop ends here
		
		return true;
	}

	/**
	  *	@desc Checks whether a given value is present in a specified row.
	  *	@param row (int) The specified row.
	  *	@param value (int) The value to be checked for.
	  *	@return (bool) Whether the value is present in the row or not.
	*/
	private:bool rowContains(int row, int value){
		int colIter;
		for(colIter=0; colIter<frame.gridLength; colIter++){
			if(frame.getCellValue(row,colIter)==value)
				return true;
		}
		return false;
	}

	/**
	  *	@desc Checks whether a given value is present in the specified column.
	  *	@param col (int) The specified column.
	  *	@param value (int) The value to be checked for.
	  *	@return (bool) Whether the value is present in the col or not.
	*/
	private:bool columnContains(int col, int value){
		int rowIter=0;
		for(rowIter=0; rowIter<frame.gridLength; rowIter++){
			if(frame.getCellValue(rowIter,col)==value)
				return true;
		}
		return false;
	}

	/**
	  *	@desc Checks whether a given value is present in the specified 3x3 grid.
	  *	@param squareNumber (int) The 3x3 grid specified. The available grids are 0-8.
	  *	@param value (int) The value to be checked for.
	  *	@return (bool) Whether the value is present or not.
	*/
	private:bool squareContains(int squareNumber, int value){
		int rowStart=(squareNumber/(int)(sqrt(frame.gridLength)))*(int)(sqrt(frame.gridLength));
		int rowEnd=rowStart+((int)(sqrt(frame.gridLength)) - 1);
		
		int colStart=(squareNumber%(int)(sqrt(frame.gridLength)))*(int)(sqrt(frame.gridLength));
		int colEnd=colStart+((int)(sqrt(frame.gridLength)) - 1);

		int rowIter, colIter;
		for(rowIter=rowStart; rowIter<=rowEnd; rowIter++){
			for(colIter=colStart; colIter<=colEnd; colIter++){
				if(frame.getCellValue(rowIter,colIter)==value)
					return true;
			}
		}

		return false;
	}

};


int main(){
	SudokuValidator s;
	return 0;
}
