/*
 * util.c

 *
 *  Created on: Feb 27, 2014
 *      Author: andrewbrooks
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "assert.h"
#include "move.h"

const U64 bitmap1 = C64(0x8080808080808080);
const U64 bitmap2 = C64(0x0101010101010101);

U64 inBetweenWithoutLookup(int sq1, int sq2);
int bitScanForward(U64 bb) ;
U64 nortFill(U64 gen) ;
U64 soutFill(U64 gen) ;

void printMoves(int *moves, int numMoves) {

	for (int i=0; i < numMoves; i++)
	{
		printMove(moves[i]);
		i++;
	}
}

int min(int x, int y) {
	int i = x < y ? x: y ;
	return i;
}
int popCount (U64 x) {
   int count = 0;
   while (x) {
       count++;
       x &= x - 1; // reset LS1B
   }
   return count;
}

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

int* getInBetweenSquares(int from, int to) {
			//get the indices of squares between and not including from and to
			int k=0, idx2;

			int * out = (int *) calloc(8, sizeof(int));

			U64 inBetweenSquares = inBetweenWithoutLookup(from, to);

			while (inBetweenSquares){
				    idx2 =bitScanForward(inBetweenSquares);
					//System.out.println("rook in between idx " + idx);
				    out[k++]=idx2;
					inBetweenSquares = inBetweenSquares & ( inBetweenSquares - 1 );
			}
			if (k > 0) {
				return out;
			}
			return 0;
		}

U64 inBetweenWithoutLookup(int sq1, int sq2)
	{
		//Gerd Isenberg
		const U64 o = 1, m1 = -o;
		const U64 a2a7 = C64(0x0001010101010100);
		const U64 b7h1 = C64(0x0002040810204080);
		const U64 b2h8 = C64(0x8040201008040200);
	    U64 btwnbits, raybits;
	    int rankDiff, fileDiff, antiDiff, diaxDiff;

	    btwnbits  = (o<<sq1) -  o;
	    btwnbits ^= (o<<sq2) -  o;
	    rankDiff  =((sq2 |7) -  sq1)>>3;
	    fileDiff  = (sq2 &7) - (sq1 &7);
	    antiDiff  = rankDiff + fileDiff;
	    rankDiff  = rankDiff &  15;
	    fileDiff  = fileDiff &  15;
	    antiDiff  = antiDiff &  15;
	    diaxDiff  = rankDiff ^ fileDiff;
	    raybits   =((rankDiff-1)>>26)*2;
	    raybits  |= (m1+fileDiff)& a2a7;
	    raybits  |= (m1+antiDiff)& b7h1;
	    raybits  |= (m1+diaxDiff)& b2h8;
	    raybits  *= btwnbits &-btwnbits;
	    return      raybits  & btwnbits;
	}
/**
 * bitScanForward
 * @author Martin LŠuter (1997)
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
int getIndexFromSquare ( char square[]) {
	assert(square != NULL);

	unsigned long len = strlen(square);
	//printf("in getIndexFromSquare; len = %lu\n", len);
	assert(len==2);
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
	static U64 soutOne (U64 b) {return  b >> 8;}
	static U64 nortOne (U64 b) {return  b << 8;}

	static U64 wFrontSpans(U64 wpawns) {return nortOne (nortFill(wpawns));}
	//U64 bRearSpans (U64 bpawns) {return nortOne (nortFill(bpawns));}
	static U64 bFrontSpans(U64 bpawns) {return soutOne (soutFill(bpawns));}
	//U64 wRearSpans (U64 wpawns) {return soutOne (soutFill(wpawns));}

	// pawns with at least one pawn in front on the same file
	//U64 wPawnsBehindOwn(U64 wpawns) {return wpawns & wRearSpans(wpawns);}

	// pawns with at least one pawn behind on the same file
	static U64 wPawnsInfrontOwn (U64 wpawns)
	{
		return wpawns & wFrontSpans(wpawns);

	}
	static U64 bPawnsInfrontOwn (U64 bpawns)
	{
		return bpawns & bFrontSpans(bpawns);

	}
	 int doubledPawnsWhite(U64 pawns) {
		return popCount(wPawnsInfrontOwn(pawns));
	}
	 int doubledPawnsBlack(U64 pawns) {
		return popCount(bPawnsInfrontOwn(pawns));
	}
	 int isolatedPawnsWhite(U64 whitePawns, U64 blackPawns)
	{
		U64 c = ((whitePawns & ~bitmap2 ) << 7);   //capture
		U64 d = ((whitePawns & ~bitmap1)  << 9 );  //capture

		U64 pawnAttacksWhite   = c | d;
		return popCount(whitePawns & ~ soutFill(nortFill(pawnAttacksWhite)));
	}

	 int isolatedPawnsBlack(U64 whitePawns, U64 blackPawns)
	{
		U64 c = ((blackPawns & ~bitmap1) >> 7 );
		U64 d = ((blackPawns & ~bitmap2) >> 9 );
		U64 pawnAttacksBlack = c | d;

		return popCount(blackPawns & ~ soutFill(nortFill(pawnAttacksBlack)));
	}

	 U64 passedPawnsWhite(U64 whitePawns, U64 blackPawns) {
				 U64  c = ((blackPawns & ~0x8080808080808080L) >> 7 );
				 U64  d = ((blackPawns & ~0x0101010101010101L) >> 9 );
				 U64  pawnAttacksBlack = c | d;

				 U64  blackdown  = soutFill(blackPawns | pawnAttacksBlack);
				 U64  whitePassers = whitePawns & ~blackdown;
				return whitePassers;
		//return popCount(whitePassers);
	}

	 U64 passedPawnsBlack(U64 whitePawns, U64 blackPawns) {
			    U64 c = ((whitePawns & ~0x0101010101010101L ) << 7);   //capture
			    U64 d = ((whitePawns & ~0x8080808080808080L)  << 9 );  //capture

			    U64 pawnAttacksWhite   = c | d;

			    U64 whiteup  = nortFill(whitePawns | pawnAttacksWhite);
			    U64 blackPassers = blackPawns & ~whiteup;
				return blackPassers;
		//return popCount(blackPassers);

	}

	//http://chessprogramming.wikispaces.com/Pawn+Fills#FileFill
	 U64 nortFill(U64 gen) {
		gen |= (gen <<  8);
		gen |= (gen << 16);
		gen |= (gen << 32);
		return gen;
	}

	 U64 soutFill(U64 gen) {
		gen |= (gen >>  8);
		gen |= (gen >> 16);
		gen |= (gen >> 32);
		return gen;
	}
	 U64  fileFill(U64 gen) {
		   return nortFill(gen) | soutFill(gen);
	}

	 U64 openFiles(U64 wpawns, U64 bpawns) {
		   return ~fileFill(wpawns | bpawns);
	}
	 U64 halfOpenOrOpenFile(U64 gen) {return ~fileFill(gen);}

	 U64 wHalfOpenFile(U64 wpawns, U64 bpawns) {
		   return halfOpenOrOpenFile(wpawns)
		        ^ openFiles(wpawns, bpawns);
	}
	 U64 bHalfOpenFile(U64 wpawns, U64 bpawns) {
		   return halfOpenOrOpenFile(bpawns)
		        ^ openFiles(wpawns, bpawns);
	}

void sort(int array[], int n) {
	 	int c, d, swap;
	 	for (c = 0 ; c < ( n - 1 ); c++){
	 		for (d = 0 ; d < n - c - 1; d++)
	 		{
	 			if (array[d] > array[d+1]) /* For decreasing order use < */
	 			{
	 				swap       = array[d];
	 				array[d]   = array[d+1];
	 				array[d+1] = swap;
	 			}
	 		}
	 	}
	 }

int dedup(int arr[], int size) {    //removes duplicates from int array
	 	sort(arr, size);
	 	int curr = 0, next = 0;
	 	while (next < size) {
	 		while (next < size && arr[next] == arr[curr])
	 			next++;
	 		if (next < size)
	 			arr[++curr] = arr[next++];
	 	}
	 	return size ? curr+1 : 0;
}
