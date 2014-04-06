/*
 * make_test.c
 *
 *  Created on: Mar 9, 2014
 *      Author: andrewbrooks
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "defs.h"
#include <assert.h>

#include "util.h"
#include "movegen.h"
#include "move.h"
#include "magic.h"
#include "eval.h"
#include "gamestate.h"


extern gameState gs;

void assertEquals(gameState gs1, gameState gs2) {
	int comp_val=memcmp(gs1.board, gs2.board,  64*sizeof(int));
	assert(comp_val==0);

	comp_val=memcmp(gs1.bitboard, gs2.bitboard,  NUMBITBOARDS*sizeof(U64));
	assert( comp_val==0);

	comp_val=memcmp(gs1.material, gs2.material,  2*sizeof(int));
	assert( comp_val==0);

	comp_val=memcmp(gs1.positional, gs2.positional,  2*sizeof(int));
	assert( comp_val==0);

	assert(gs.flags==gs2.flags);
	assert(gs.hash==gs2.hash);
	assert(gs.color==gs2.color);

	comp_val=memcmp(gs1.W_CASTLING_RIGHTS, gs2.W_CASTLING_RIGHTS,  4*sizeof(U64));
	assert( comp_val==0);

	comp_val=memcmp(gs1.B_CASTLING_RIGHTS, gs2.B_CASTLING_RIGHTS,  4*sizeof(U64));
	assert( comp_val==0);
}
void setEPSquareOnDoubleMove() {
	printf("Running setEPSquareOnDoubleMove\n");
	char fen[] = "2b2rk1/1p2p2p/1p2n1p1/r2BNp2/3p4/4P3/PPP2PPP/R4RK1 w - - 1 19";
	parseFen(fen);
	MOVE move = createMoveFromString("c2c4", WP, 0, doublePawnMove);
	make(move);
	int epSquare = getEPSquare();
	printf("epSquare %d\n", epSquare);
	assert(epSquare != 0);
}

void testMaterial() {
	int x =  materialValue(WHITE) + materialValue(BLACK);
	int y = gs.material[WHITE] + gs.material[BLACK];

	assert(x==y);
}

/*void testCastlingPrivilege() {
	printf("Running testCastlingPrivilege\n");
		char fen[] = "rnbqk1nr/ppppppbp/6p1/8/7P/1P6/P1PPPPP1/RNBQKBNR b KQkq - 0 3";
		parseFen(fen);
		int move = createMoveFromString("g7-a1", BB, WR, captureNoPromotion);
		make(move);
		//testMaterial();
	}*/
void capturePromotionTestBlack() {
	    printf("capturePromotionTestBlack\n");
		char s[] = "rnbqkbnr/pp1ppppp/7P/8/8/8/PPp1PPP1/RNBQKBNR b KQkq - 0 5";
		char textMove[]= "c2d1";
		int myFromType=BP;
		int victim=WQ;

		parseFen(s);
		int flags = gs.flags;
		long hash = gs.hash;
		gameState gs2=gs;

		MOVE move = createMoveFromString(textMove, myFromType, victim, capturePromotionQueen);
		make(move);
		unmake(move, flags, hash);
		assertEquals(gs2,gs);
		testMaterial();

	}
void enPassantTestBlack2() {
	//en pssant is illegal here
	printf("enPassantTestBlack2\n");
	char fen[] = "8/8/8/8/R1pP2k1/4P3/P7/K7 b - d3 0 1";
	parseFen(fen);
	char textMove[] = "c4d3";
	int myFromType=BP;
	//DO AN ENPASSANT CAPTURE, SHOULD FAIL
	MOVE move = createMoveFromString(textMove, myFromType, WP, epCapture);

	make(move);
	//gs.print();
	bool inCheck = isInCheckAux( gs.bitboard[BK],  WHITE);
	assert(true==inCheck);
}
void simplePromotionTestWhite() {
	char s[] = "rnbqkb1r/1p1pppPp/p4n2/8/8/8/PP2PPP1/RNBK1BNR w kq - 1 8";
	char textMove[]= "g7g8q";
	int myFromType=WP;
	int victim=0;
	parseFen(s);


	gameState gs2 = gs;
	int flags = gs.flags;
	long hash = gs.hash;


	MOVE move = createMoveFromString(textMove, myFromType, victim, simplePromotionQueen);
	make(move);
	unmake(move, flags, hash);
	assertEquals(gs2,gs);
	testMaterial();
}
void make_test() {

	printf("Running make_test\n");
	char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	parseFen(fen);
	int numMoves;
	MOVE movelist[MAX_MOVES];
	generateNonCaptures(gs.color, movelist, &numMoves, gs.bitboard);

	gameState gs2= gs;

	int flags = gs.flags;
	U64 hash2 = gs.hash;

	for (int i=0; i < numMoves; i++)
	{
		MOVE move = movelist[i];
		//printf("doing make/unmake on move  %s\n",  moveToString(move));
		make(move);
		unmake(move, flags, hash2);

		assertEquals(gs, gs2);

	}

}
void bishopCheck() {
    	char s[] = "2kr1bnr/ppp2ppp/8/4n3/6b1/2P2N2/PPK1BPPP/RNB4R b - - 3 9";
    	printf("Running bishop_check\n");

    	MOVE move = createMoveFromString("g4f5", BB, 0, simple);
    	int flags2 = gs.flags;
		long hash2 = gs.hash;
		parseFen(s);
		gameState gs2=gs;
		make(move);

		unmake(move, flags2, hash2);

		assertEquals(gs2, gs);
		testMaterial();
    }
void promotionTestWhite() {
	printf("Promotion Test white\n");
	char fen[] = "5B2/6P1/1p6/8/1N6/kP6/2K5/8 w - - 0 1";
	parseFen(fen);

	char moveStr[]= "g7g8q";
	MOVE move = createMoveFromString(moveStr, WP, 0, simplePromotionQueen);
	make(move);
	int cnt = popCount(gs.bitboard[WQ]);
	assert(cnt==1);
}
void make_test_suite() {
	assert(gs.initialized);
	make_test();
	bishopCheck();
	//testCastlingPrivilege();
	promotionTestWhite();
	capturePromotionTestBlack();
	setEPSquareOnDoubleMove();
	enPassantTestBlack2();
}
