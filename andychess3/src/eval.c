/*
 * eval.c
 *
 *  Created on: Mar 2, 2014
 *      Author: andrewbrooks
 */
#include "stdbool.h"

const U64 kingsideMask[] =  { C64(67569408), C64(1978038598238208)}; // white, black

const U64 queensideMask[] = { C64(551608320), C64(63296822826762240)}; // white, black

const U64 whiteKSpawnShields[] = { C64(57344), C64(132352), C64(67072), C64(262912),  C64(67109632)  };	 //best, fianchetto, best w/h3, best w/f3, best w/f4

const U64 whiteQSpawnShields[] = { C64(1792), C64(8413184), C64(4235264), C64(2146304),  C64(536920064)  };	 //best, fianchetto, best w/a3, best w/c3, best w/c4

const U64 blackKSpawnShields[] = { C64(1970324836974592), C64(1409573906808832), C64(1689949371891712),
		C64(848822976643072),  C64(844442110001152)  };	 //best, fianchetto, best w/h3, best w/f3, best w/f4

const U64 blackQSpawnShields[] = { C64(63050394783186944), C64(45106365017882624), C64(27162335252578304), C64(54078379900534784),  C64(54043332967399424)  };	 //best, fianchetto, best w/a3, best w/c3, best w/c4

bool goodKingsideShield(U64 x, int color) {

	if (color==0) {
		for (int i=0; i < sizeof(whiteKSpawnShields); i++ )
			if (x == whiteKSpawnShields[i]) return true;
	}
	else {
		for (int i=0; i < sizeof(blackKSpawnShields); i++ )
			if (x == blackKSpawnShields[i]) return true;
	}
	return false;
}

bool goodQueensideShield(U64 x, int color) {

	if (color==0) {
		for (int i=0; i < sizeof(whiteQSpawnShields); i++ )
			if (x == whiteQSpawnShields[i]) return true;
	}
	else {
		for (int i=0; i < sizeof(blackQSpawnShields); i++ )
			if (x == blackQSpawnShields[i]) return true;
	}
	return false;
}
const short queenTable[] =  {
		//queen
		-20,-10,-10, -5, -5,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5,  5,  5,  5,  0,-10,
		-5,  0,  5,  5,  5,  5,  0, -5,
		-5,  0,  5,  5,  5,  5,  0, -5,
		-10,  5,  5,  5,  5,  5,  0,-10,
		-10,  0,  5,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20
};
short blackQueenTable[64];


const short whitePawnTable[] =
{
		0,  0,  0,  0,  0,  0,  0,  0,
		25, 25, 25, 25, 25, 25, 25, 25,
		22, 22, 22, 22, 22, 22, 22, 22,
		5,  5, 10, 20, 20, 10,  5,  5,
		0,  0,  0, 20, 20,  0,  0,  0,
		5, -5,- 5,  0,  0, -5, -5,  5,
		5, 10, 10,-25,-25, 10, 10,  5,
		0,  0,  0,  0,  0,  0,  0,  0
};

short blackPawnTable[64];


//Knights are encouraged to control the center and stay away from edges to increase mobility:

const short knightTable =
{
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-30,  5, 10,  5,  5, 10,  5,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  5, 10,  5,  5, 10,  5,-30,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50,
};

short blackKnightTable[64];



//Bishops are also encouraged to control the center and stay away from edges and corners:

short knighttables[][]  = { knightTable, blackKnightTable};

const short whiteBishopTable =
{
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10, -5,  5, 10, 10,  5, -5,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10, 10,  0,  0,  0,  0, 10,-10,
		-20,-10,-20,-10,-10,-20,-10,-20,
};

short  blackBishopTable[64];

short bishoptables[][] = { whiteBishopTable, blackBishopTable};

const short kingTableEndGame =
{
		-50,-40,-30,-20,-20,-30,-40,-50,
		-20,-20,-10,  0,  0,-10,-20,-20,
		-20,-10, 20, 30, 30, 20,-10,-20,
		-20,-10, 30, 40, 40, 30,-10,-20,
		-20,-10, 30, 40, 40, 30,-10,-20,
		-20,-10, 20, 30, 30, 20,-10,-20,
		-20,-20, -10, 0,  0,-10,-20,-20,
		-50,-40,-30,-20,-20,-30,-40,-50
};


short pieceTables[][] = {  whitePawnTable, blackPawnTable, knightTable, blackKnightTable,
		whiteBishopTable, blackBishopTable};

short queenPieceTables[][] = {  queenTable, blackQueenTable };

void initializeEval() {

	for (int i = 0; i < 64; i++) {
		blackQueenTable[i] = queenTable[63-i];
		blackPawnTable[i] = whitePawnTable[63-i];
		blackKnightTable[i] = knightTable[63-i];
		blackBishopTable[i] = whiteBishopTable[63-i];
	}
}
