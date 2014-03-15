/*
 * defs.h
 *
 *  Created on: Feb 26, 2014
 *      Author: andrewbrooks
 */
#include "stdbool.h"

#ifndef DEFS_H_
#define  DEFS_H_

#define NDEBUG

typedef unsigned long long U64 ;
#define C64(constantU64) constantU64##ULL

#define WHITE 0
#define BLACK 1

#define WP 0
#define BP 1
#define WN 2
#define BN 3
#define WB 4
#define BB 5
#define WR 6
#define BR 7
#define WQ 8
#define BQ 9
#define WK 10
#define BK 11
#define EMPTY -1

#define WHITEPIECES 12
#define BLACKPIECES 13
#define ALLPIECES 14

#define NUMBITBOARDS 15

#define  MAX_PLIES 200

#define  WKSIDE 0
#define  WQSIDE 1
#define  BKSIDE 2
#define  BQSIDE 3

#define   EP_SHIFT  25
#define   EP_CLEAR  0x1FFFFFF

#define  QUEEN_COUNT_BONUS  20
#define  KING_WALK_PENALTY -30
#define  WING_PAWN_PUSH_PENALTY  -25
#define  ROOK_COUNT_BONUS  20
#define  CASTLING_BONUS  30
#define  ROOK_ON_SEVENTH_BONUS  20
#define  WING_PAWN_PUSH_BONUS  20
#define  TWO_BISHOPS_BONUS  15

//#define  ISOLATED_PAWN_PENALTY  -20
#define  PAWN_REMOVAL_BONUS  7
//#define  ROOK_OPEN_OR_HALF_OPEN_FILE_BONUS  15

#define  ENDGAME_MATERIAL_VALUE  3000

#define  QUEEN_VALUE  985  // slightly higher since they don't use pos. tables
#define  ROOK_VALUE  515  // slightly higher since they don't use pos. tables
#define  BISHOP_VALUE  350
#define  KNIGHT_VALUE  325
#define  PAWN_VALUE  100

#define PAWN_STRUC_TBL_ENTRIES 500000
#define TRANS_TBL_ENTRIES 5000000

#define NUM_BOOK_MOVES 3072
#define MAX_MOVES   200// from a given board position
#define MAX_QUIES_MOVES 100


typedef struct BookMove {
	U64 hash;
	int movelist[10];
} BookMove;

typedef struct MoveInfo {
	int move;
	int value;
} MoveInfo;
typedef int (*compfn)(const void*, const void*);

typedef  struct transpositionTable {

	U64 hashValue;
	int bestmove;
	int depthValue;
	char flags;
	int nodeScore;

} transpositionTable;

typedef  struct pawnStructureTable {

	 U64 pawnHash;
	 U64 whitePawns;
	 U64  blackPawns;
	 int whiteScore;
	 int blackScore;

} pawnStructureTable;

typedef struct gameState {
	int 			board[64];
	U64 			bitboard[NUMBITBOARDS];

	int             color;  //player to move
	int 			flags;
	U64 			hash;
	int				material[2];
	int         	positional[2];

	//used for extensions
	bool      		promotion;
	bool      		seventhRankExtension;
    int 			moveHistory[MAX_PLIES];
	int 			capturedPieces[MAX_PLIES];
	int   			moveCounter;

	U64 			pieces[12][64] ;  	// [piece type][square]
	U64 			en_passant[64] ;   	 	// En passant
	U64   			side;    // Used for changing sides
	U64  			W_CASTLING_RIGHTS[4];   // both ways, short, long and none  0,1,2,3
	U64  			B_CASTLING_RIGHTS[4];   // both ways, short, long and none

    int 			currentPly;
    bool 			initialized;
} gameState;
#endif /* DEFS_H_ */
