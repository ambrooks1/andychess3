/*
 * util.h
 *
 *  Created on: Feb 28, 2014
 *      Author: andrewbrooks
 */

#ifndef UTIL_H_
#define UTIL_H_

int dedup(MOVE arr[], int size);
int min(int x, int y);
int bitScanForward(U64 bb);
int convertPiece(char c);
char reverseConvertPiece(int i);
void printMoves(MOVE *moves, int numMoves);
int getIndexFromSquare ( char *square);
const char* getSquareFromIndex(int idx);

U64 setBitLong(U64 x, int position);
int setBit(int x, int position);
int clearBit(int x, int position);
U64 inBetweenWithoutLookup(int sq1, int sq2);
int popCount (U64 x);
int* getInBetweenSquares(int from, int to);
U64 passedPawnsWhite(U64 whitePawns, U64 blackPawns) ;
U64 passedPawnsBlack(U64 whitePawns, U64 blackPawns) ;

#endif /* UTIL_H_ */
