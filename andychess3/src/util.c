/*
 * util.c

 *
 *  Created on: Feb 27, 2014
 *      Author: andrewbrooks
 */

#include "defs.h"
#include "assert.h"
#include <string.h>

const int index64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

/**
 * bitScanForward
 * @author Martin L�uter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 * "Using de Bruijn Sequences to Index a 1 in a Computer Word"
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
int bitScanForward(U64 bb) {
   const U64 debruijn64 = C64(0x03f79d71b4cb0a89);
   assert (bb != 0);
   return index64[((bb & -bb) * debruijn64) >> 58];
}

const char pieces[12] = { 'P', 'p', 'N', 'n', 'B', 'b', 'R', 'r', 'Q', 'q','K','k' };

char reverseConvertPiece(int i) {
	assert((i >=0 && i <=11) || i == -1);
	switch(i) {
		case EMPTY : return '-';
		default : return pieces[i];
	}
}


int convertPiece(char c) {
	switch (c) {
			case 'P' : return WP ;
			case 'p' : return BP ;

			case 'N' : return WN;
			case 'n' : return BN;

			case 'B' : return WB;
			case 'b' : return BB;

			case 'R': return WR;
			case 'r': return BR;

			case 'Q': return WQ;
			case 'q': return BQ;

			case 'K': return WK;
			case 'k': return BK;
			}
			return EMPTY;
}

const char *squares[64] = {
				"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
				"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
				"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
				"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
				"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
				"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
				"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
				"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
				};

const char* getSquareFromIndex(int idx) {
	assert(idx >=0 && idx <= 63);
	return squares[idx];
}
int getIndexFromSquare ( char *square) {
	for (int i=0; i < 64; i++) {
		if (strcmp(square, squares[i]) == 0) {
			return i;
		}
	}
	return -1;
}


int setBit(int x, int position)
{
	int mask = 1 << position;
	return x | mask;
}
int clearBit(int x, int position)
{
	int mask = 1 << position;
	return x &  ~mask;
}
U64 setBitLong(U64 x, int position)
{
	U64 mask = C64(1) << position;
	return x | mask;
}
