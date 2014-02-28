/*
 * util.h
 *
 *  Created on: Feb 28, 2014
 *      Author: andrewbrooks
 */

#ifndef UTIL_H_
#define UTIL_H_

int bitScanForward(U64 bb);
int convertPiece(char c);
char reverseConvertPiece(int i);

int getIndexFromSquare ( char *square);
const char* getSquareFromIndex(int idx);

U64 setBitLong(U64 x, int position);
int setBit(int x, int position);
int clearBit(int x, int position);

#endif /* UTIL_H_ */
