/*
 * move.c
 *
 *  Created on: Mar 1, 2014
 *      Author: andrewbrooks
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "move.h"
#include "defs.h"
#include "util.h"

void printMove(int move) {
	char s[5];
	moveToString(move, s);
	printf( "%s\n", s);

}
/**
	 *  @return int Piece moving
	 */
	 int pieceMoving(int move)
	{
		return ((move >> PIECE_SHIFT) & PIECE_MASK) ;
	}
	// END pieceMoving()

	/**
	 *  @return int To-index
	 */
	 int toIndex(int move)
	{
		return ((move >> TO_SHIFT) & SQUARE_MASK);
	}
	// END toIndex()

	/**
	 *  @return int From-index
	 */
	 int fromIndex(int move)
	{
		return (move & SQUARE_MASK); // Since the from-index is first in the integer it doesn't need to be shifted first
	}
	// END fromIndex()

	/**
	 *  @return int Piece captured
	 */
	 int capture(int move)
	{
		return ((move >> CAPTURE_SHIFT) & PIECE_MASK) ;
	}
	// END capture()

	/**
	 *  @return int Move type
	 */
	 int moveType(int move)
	{
		return ((move >> TYPE_SHIFT) & TYPE_MASK);
	}
	// END moveType()

	/**
	 *  @return int Ordering value
	 */
	 int orderingValue(int move)
	{
		return (move >> ORDERING_SHIFT); // Since the ordering value is last in the integer it doesn't need a mask
	}
	// END orderingValue()

	/**
	 *  Clears the ordering value and sets it to the new number
	 *
	 *  Important: Ordering value in the move integer cannot be >127
	 *
	 *  @param move The move to change
	 *  @param value The new ordering value
	 *  @return move The changed moved integer
	 */
	 int setOrderingValue(int move, int value)
	{
		move = (move & ORDERING_CLEAR); // Clear the ordering value
		return (move | (value << ORDERING_SHIFT)); // Change the ordering value and return the new move integer
	}
//fromType, fromIndex, toIndex, capture, moveType, 0);
	/**
	 *  Creates a move integer from the gives values
	 *
	 *  @param pieceMoving
	 *  @param fromIndex
	 *  @param toIndex
	 *  @param capture
	 *  @param type
	 *  @param ordering If we want to assign an ordering value at creation time, probably won't be used much for now
	 *  @reutrn move The finished move integer
	 */
	 int createMove(int pieceMoving, int fromIndex, int toIndex, int capture, int type, int ordering)
	{
		//if (type > 16) System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

		int move = 0
		    | fromIndex	// from
			| (toIndex << TO_SHIFT) // to
			| (pieceMoving  << PIECE_SHIFT) // piece moving
			| (capture << CAPTURE_SHIFT) //piece captured
			| (type << TYPE_SHIFT) // move type
			| (ordering << ORDERING_SHIFT); // ordering value
		return move;
	}
	 int createMoveFromString(char str[], int fromType, int capture, int moveType2)
	//like e2-e4   used in testing only
	{
		   // printf("start createMoveFromString\n");
		    const char s[] = "-";
		    char *token1, *token2;

		    token1 = strtok(str, s);
		    token2 = strtok(NULL, s);
		    if (token1 == NULL || token2 == NULL)
		    	return 0;

			int fromIndex=getIndexFromSquare(token1);
			int toIndex= getIndexFromSquare(token2);

			return createMove(fromType, fromIndex, toIndex, capture, moveType2, 0);
	}
	// END createMove
	/**
	 * Gets the file of the given board[] index
	 *
	 * @param   index   array index of position [0-63]
	 * @return  file  of the position ['a'-'h']
	 */
	char toFile(int index)
	{
		char file = 'a';
		file += index %  8; // += 0-7
		return file;
	}


	/**
	 * Gets the rank of the given board[] index
	 *
	 * @param   index   array index of position [0-63]
	 * @return  rank of the position [1-8]
	 */
	int toRank(int index)
	{
		return (8 - index/8); // 1-8
	}

	 char promotionConvert(int type, int pieceMoving)
		{

			if (pieceMoving==WP) {
				switch(type) {
				case capturePromotionQueen: case simplePromotionQueen : return 'Q';
				default : return ' ';
			    }

			}
			else {
				switch(type) {

				case capturePromotionQueen: case simplePromotionQueen : return 'q';
				default : return ' ';
			    }

			}
		}
	void moveToString(int move, char s[]) {


		strcpy(s, getSquareFromIndex(fromIndex(move)));
		strcat(s, getSquareFromIndex(toIndex(move)));

		int type = moveType(move);
		int pm = pieceMoving(move);

		if (type == simplePromotionQueen || type == capturePromotionQueen) { //promotion
			s[4] =  promotionConvert(type, pm);
		}

	}



