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
#include <assert.h>

bool moveEqual(MOVE x, MOVE y) {

	int i = memcmp(&x, &y, sizeof(MOVE));
	if (i==0) return true;
	return false;
	//if this does not work, try a field by field equals test
}
void printMove(MOVE move) {
	char s[MOVE_STR_SIZE];
	moveToString(move, s);
	printf( "%s\n", s);

}

MOVE createMove(int pieceMoving, int fromIndex, int toIndex, int capture, int type, int ordering)
{
	MOVE move={0};
	move.pieceMoving=pieceMoving;
	move.fromIndex=fromIndex;
	move.toIndex=toIndex;
	move.capture=capture;
	move.type=type;
	move.orderVal=ordering;
	return move;
}

bool isMoveString(char command[]) {   // from CPW Engine
	if (command[0] >= 'a' && command[0] <= 'h' &&
		command[1] >= '1' && command[1] <= '8' &&
		command[2] >= 'a' && command[2] <= 'h' &&
		command[3] >= '1' && command[3] <= '8' &&

			( command[4] == ' ' || command[4] == '\n' || command[4] == 0 ||
			 command[4] == 'q' ||   command[4] == 'r' ||  command[4] == 'b' || command[4] == 'n'

					 ) )
		return true;
	return false;

}
MOVE createMoveFromString(char moveStr[], int fromType, int capture, int moveType2){
	assert(isMoveString(moveStr));
	char fromSquare[3], toSquare[3];
	fromSquare[0]= moveStr[0];
	fromSquare[1]= moveStr[1];
	fromSquare[2]= '\0';

	toSquare[0]= moveStr[2];
	toSquare[1]= moveStr[3];
	toSquare[2]= '\0';

	int fromIndex=getIndexFromSquare(fromSquare);
	int toIndex= getIndexFromSquare(toSquare);

	return createMove(fromType, fromIndex, toIndex, capture, moveType2, 0);
}


void moveToString(MOVE move, char s[]) {
	strcpy(s, getSquareFromIndex(move.fromIndex));
	strcat(s, getSquareFromIndex(move.toIndex));

	int type = move.type;
	if (type == simplePromotionQueen || type == capturePromotionQueen) { //promotion
		strcat(s, "q");
	}
}
