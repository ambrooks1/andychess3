/*
 * tester.c
 *
 *  Created on: Mar 6, 2014
 *      Author: andrewbrooks
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "assert.h"
#include "defs.h"
#include "util.h"
#include "movegen.h"
#include "move.h"
#include "magic.h"
#include "eval.h"
#include "gamestate.h"
#include "perft_test.h"
#include "timeControl.h"
#include "search.h"
#include "make_test.h"
#include "SEE_test.h"
#include "book.h"
#include "engine2.h"

extern gameState gs;
extern bool search_debug;
U64 total=0;

void run_search( char *fen, char *answer) ;


void hash_test_aux(char moveStr[], int pieceMoving, int victim, int myMoveType) {
	int move = createMoveFromString(moveStr, pieceMoving, victim, myMoveType);
	if (move != 0)
		make(move);
}
void hash_test() {
	printf("hash_test()\n");
	char startPos[] ="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	parseFen(startPos);
	U64 hash = gs.hash;

	hash_test_aux("g1f3", WN, 0, simple);
	hash_test_aux("g8f6", BN, 0, simple);
	hash_test_aux("f3g1", WN, 0, simple);
	hash_test_aux("f6g8", BN, 0, simple);

	U64 hash2 = gs.hash;

	printf("hash %lld hash2 %lld\n", hash, hash2);
	assert(hash==hash2);

	printf("end hash_test()\n");
}

void check_evasion_aux(char *fen, int i) {
	//printf("fen= %s\n", fen);

	//printf("check evasion moves\n");
	parseFen(fen);
	int cntMoves;
	int movelist[200];
	getLegalCheckEvasions( movelist, &cntMoves);
	/*for (int i=0; i < cntMoves; i++) {
		printMove(movelist[i]);
	}*/
	//printf("legal moves\n");
	parseFen(fen);
	int cntMoves2;
	int movelist2[200];
	getLegalMoveList( movelist2, &cntMoves2);
	/*for (int i=0; i < cntMoves2; i++) {
			printMove(movelist2[i]);
	}*/
	//printf("i= %d number of legal check evasion moves: %d   number of legal moves %d\n", i, cntMoves, cntMoves2);

	assert(cntMoves==cntMoves2);
}


void check_evasion_test() {
	char * s[] = {
			"2Nr4/pp2k3/1nn2R1p/3p4/2pP2Q1/2P5/PPB4P/R5K1 b - -",
			"2rr3k/pp3p2/1nnqbpNp/3p4/2pP4/2P3Q1/PP5P/R2B1RK1 b - -",
			"2rr3k/pp3p2/1nnqb2p/3pQ3/2pP4/2P5/PPB2K1P/R3R3 b - -",
			"2rrNq1k/pp3pp1/1nn1b1Np/3p4/3P4/2Pp4/PPB4P/R4RK1 b - -",
			"2rr2Qk/pp3p2/1nq1bp1p/3p4/2pP4/2P5/PPB2K1P/R4R2 b - -",
			"2rQ3k/pp3pp1/1n1qb2p/3p4/2pP2n1/2P5/PPB4P/R4RK1 b - -",
			"2rr3k/pp3N2/1nnqb1Bp/3p2p1/2pP4/2P5/PP5P/R4RK1 b - -",
			"2rr3k/pp3N2/1nnqbppp/3p4/2pP4/2P4P/PPB5/R4RK1 b - -",
			"2rr2Qk/pp3p2/1nnqb2p/3pp3/2pP3P/2P2R2/PPB5/R5K1 b - -",
			"2rr1k2/pp6/1nnQ1pR1/3p4/2pP2b1/2P5/PPB4P/R5K1 b - -",
			"2rr3k/pp3p2/1nnqbQ1p/3pp3/2pP4/2P5/PPB4P/3R1RK1 b - -",
			"2rr3k/pp3pQ1/1nnq1p1p/3pNb2/2pP4/2P5/PP5P/R4RK1 b - -",
			"2r3rk/pp3p2/1nnqb2p/3pp3/P1pP4/2P5/1PB3KP/R4R2 w - -",
			"2r3rk/pp3p2/1nnqb2p/3pp3/2pP4/2P5/PP5P/RBR3K1 w - -",
			"2rr2Qk/pp3p2/1nnqb2p/3pp3/2pP1R2/2P5/PPB3KP/R7 b - -",
			"2r3rk/pp3p2/2nqbp1p/3pN3/n1pP4/2P5/PP5P/R2R2K1 w - -",
			"2rr3k/pp3p2/1nnqb2p/3pQ3/2pP4/2P5/PPB1R2P/R5K1 b - -",
			"2rr3k/pp3p2/1nnqb2p/3pQ3/2pP4/2P5/PP5P/RR1B2K1 b - -",
			"2r3rk/pp3p2/1nnqbp1p/3pNR2/3P4/1pP5/PP5P/R5K1 w - -",
			"2r3rk/pp3p2/1nnqbp1p/3pN3/3P4/2Pp4/PP5P/4RRK1 w - -",
			"2rr3k/pp3b2/1nnq1pNp/3p4/2pP4/2P5/PPB4P/R4RK1 b - -",
			"2rrN2k/pp4p1/1nN1b1pp/3p4/2pP4/2P3Q1/PP5P/R4qK1 w - -",
			"rnb1kbnr/pppp1ppp/4p3/8/6Pq/2N2P2/PPPPP2P/R1BQKBNR w KQkq -",
			"rnb1k1nr/pppp1ppp/3bp3/8/6P1/4PP2/PPPPKq1P/RNBQ1BNR w kq -",
			"rnb1kbnr/pppp1pp1/4p3/7P/8/4PPq1/PPPP3P/RNBQKBNR w KQkq -",
			"rnb1kb1r/p1pp1ppp/4p2n/1N6/6Pq/5P2/PPPPP2P/R1BQKBNR w KQkq -",
			"r1b1kbnr/pppp1ppp/2n1p3/8/6P1/4PP2/PPPPK2q/RNBQ1BNR w kq -",
			"rnbqkbnr/ppN2ppp/4p3/8/6P1/5P2/PPPPP2P/R1BQKBNR b KQkq -",
			"rnb1kbnr/pppp1ppp/4p3/8/6P1/4PP2/PPPPK2P/RNB1qBNR w kq -",
			"r1b1kbnr/pppn1ppp/3pp3/8/6Pq/2N1PP2/PPPP3P/R1BQKBNR w KQkq -",
			"rnbqk1nr/p1pB1pp1/4p2p/8/1b4P1/4PP2/PPPP3P/RNBQK1NR b KQkq -",
			"rnbqk1nr/pppp1p2/4p1pB/8/6P1/3P1P2/PPPbP2P/R2QKBNR w KQkq -",
			"rnb1kbnr/1ppp1ppp/p3p3/8/6P1/3PqP2/PPPN3P/R1BQKBNR w KQkq -",
			"rnb1kbnr/pppp1ppp/4p3/8/6P1/3PqP2/PPPN3P/R1BQKBNR w KQkq -",
			"rnb1k1nr/pppp1ppp/4p3/8/1b4Pq/3P1P2/PPPKP2P/RNBQ1BNR w kq -",
			"r1bqk1nr/pppp1pp1/n3p3/7P/1b6/3P1P2/PPP1P2P/RNBQKBNR w KQkq -",
			"rnbqk1nr/pppp1p1p/4p3/6p1/6P1/2bPPP2/PPP4P/R1BQKBNR w KQkq -",
			"rnb1kb1r/pp1p1ppp/4pn2/q1p5/6P1/3PPP2/PPP4P/RNBQKBNR w KQkq -",
			"rnbqk1nr/ppN2p1p/4p1p1/2b5/6P1/5P2/PPPPP2P/R1BQKBNR b KQkq -",
			"rnb1kbnr/pp1p1p1p/2p1p3/8/6P1/3PqP2/PPP4P/RN1QKBNR w KQkq -",
			"rnbqk1nr/pppp1ppp/8/4p3/6P1/3PPP2/PPPb3P/R1BQKBNR w KQkq -",
			"r1b1kbnr/ppp2ppp/2n1pN2/8/6P1/5P2/PPPPP2P/R1BQKBNR b KQkq -",
			"r1b1kbnr/pppp1ppp/4p3/8/1n4P1/3PPP2/PPPK3q/RNBQ1BNR w kq -",
			"r1bqkbnr/1ppp1ppp/4p3/p7/6P1/3nPP1N/PPP4P/RNBQKB1R w KQkq -",
			"rnbqkbnr/ppN2p1p/8/4p1p1/6P1/5P2/PPPPP2P/R1BQKBNR b KQkq -",
			"rnb1k1nr/ppp2ppp/4pN2/2b5/6P1/5P2/PPPPP2P/R1BQKBNR b KQkq -",
			"rnb1kbnr/ppp3pp/4p3/5P2/3q4/3PKP2/PPP4P/RNBQ1BNR w kq -",
			"rnb1k1nr/1pp2ppp/p3p3/3p4/1b4Pq/3PPP2/PPPN3P/R1BQKBNR w KQkq -",
			"4k1r1/1b6/pq4p1/5n2/1p4P1/4QP2/PPP4P/1K6 b - -",
			"r4kr1/5Rn1/pq4p1/4Q3/1p4P1/5b2/PPP4P/1K3R2 b - -",
			"r4kr1/1b2R3/p5p1/5n2/1p3QP1/5P2/PPP4P/1K4q1 w - -",
			"r4kr1/4R1n1/p5p1/4Q1P1/1p6/5b2/PPP4P/1K4q1 w - -",
			"r5r1/4k1n1/pq4p1/8/1p2R1P1/1P3P2/P1P4P/1K6 b - -",
			"4k1r1/8/pq4p1/4Qn2/1p4P1/1P3b2/P1P4P/1K6 b - -",
			"r3Qkr1/3b2n1/pq4p1/8/1p3PP1/8/PPP4P/1K2R3 b - -",
			"r4k2/6r1/pqb2Qp1/8/1p4P1/5P2/PPP4P/1K3R2 b - -",
			"r4kr1/4R1n1/p5p1/q7/1p4P1/5R2/PPP4P/1K6 b - -",
			"r5r1/6n1/p4kp1/8/1p4P1/5R2/PPP4P/1K6 b - -",
			"r7/5kr1/pq4p1/8/1p4P1/5R2/PPP4P/1K6 b - -",
			"4k1r1/8/pq4p1/5n2/1p2Q1P1/5b2/PPP4P/1K6 b - -",
			"r3Rkr1/1b6/pq4p1/5n2/6P1/2p1RP2/PPP4P/1K6 b - -",
			"r4kr1/1bn5/pq4p1/8/1p3QP1/5P2/PPP4P/1K2R3 b - -",
			"4rkr1/1b2Q1n1/pq4p1/8/6P1/2p2P2/PP5P/1K2R3 b - -",
			"r4kr1/1b2R1Q1/pq4p1/8/1pP3P1/4nP2/PP5P/1K6 b - -",
			"r4kr1/1b3R2/pq4p1/4QnP1/8/p4P2/1PP4P/1K2R3 b - -",
			"r4kr1/2q2Rn1/p5p1/8/1p4P1/5b1P/PPP5/1K2R3 b - -",
			"r4kr1/5R2/pq4p1/4Q3/1p4P1/8/PPP4P/1K5b b - -",
			"r4kr1/4R1n1/p2q2p1/5Q2/1p4P1/5b2/PPP4P/1K6 b - -",
			"r5r1/5kn1/pq4p1/3Q4/1p4P1/5b2/PPP1R2P/1K6 b - -",
			"r4kr1/1b2R3/pq3Qp1/5n2/6P1/p4P2/1PP1R2P/1K6 b - -",
			"3r1kr1/8/p4Rp1/8/1p4P1/8/PPP4P/1K6 b - -",
			"r2k2Q1/1b6/p5p1/8/1p4P1/5P2/PPP4P/1K2R3 b - -",
			"r4kr1/3R4/pq3Qp1/5n2/1p4P1/1P3b2/P1P4P/1K2R3 b - -",
			"r4kr1/1b4n1/pq4p1/2Q5/1p4P1/8/PPP4P/1K2R3 b - -",
			"r5r1/1b2k3/pq2n1p1/6Q1/1p4P1/5P2/PPP4P/1K2R3 b - -",
			"r5r1/1bQ2k2/pq4p1/5n2/1p4P1/5P2/PPP4P/1K2R3 b - -",
			"r4kr1/4R1n1/pq3Qp1/8/1p4b1/8/PPP4P/1K2R3 b - -",
			"r4kr1/1b6/pq4pQ/8/1p4P1/4nP2/PPP4P/1K2R3 b - -",
			"r3Q3/5k2/pq4r1/8/1p6/5b2/PPP4P/2K1R3 b - -",
			"r4kr1/1b4n1/pq4p1/2Q5/1p4PP/8/PPP5/1K2R3 b - -",
			"4Rkr1/1b6/p4qp1/5n2/1p4PP/5P2/PPP5/1K6 b - -",
			"4rkr1/1b2Q3/pq4p1/8/1p4P1/2P2P2/PP5P/1K2R3 b - -",
			"r4kr1/5Rn1/pq4p1/4Q3/1p4P1/1P3b2/P1P4P/1K2R3 b - -",
			"r4kr1/1b3R2/pq4p1/4Q3/1p4Pn/1P3P2/P1P5/1K2R3 b - -",
			"r3Rkr1/1b4n1/pq4p1/8/6P1/5P2/1pP1Q2P/1K2R3 b - -"

	};

	for (int i=0; i < 85; i++)
		check_evasion_aux(s[i],i);
	//printf("%s\n",s[i]);
}

static char *eval_test() {
	assert(gs.initialized);
	printf("Running eval_test\n");
	char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	parseFen(fen);
	assert(gs.material[0]==gs.material[1]);
	assert(gs.material[0]==4165);
	printf(" material white %d material black %d\n", gs.material[0],gs.material[1]);
	printf(" positional white %d positional black %d\n", gs.positional[0],gs.positional[1]);

	int eval = getEvaluation(gs);
	printf("evaluation= %d\n", eval);
	assert( eval==0);
	return 0;
}
static char*  isLegal_test() {
	assert(gs.initialized);
	printf("Running isLegal_test\n");
	char fen[] = "rn1qkbnr/ppp2pp1/3p4/4p2p/4P2P/3P3b/PPP2PP1/RNBQKBNR w KQkq - 0 1";
	parseFen(fen);
	int numMoves;
	int moves[200];
	generateNonCaptures(gs.color, moves, &numMoves);

	for (int i=0; i < numMoves; i++)
	{
		int move = moves[i];
		//printf("Testing move ");
		//printMove(move);
		if (!isMoveLegal(move)) {
			printf("Not legal ");
			printMove(move);
			isMoveLegal(move);
		}
		assert( isMoveLegal(move));
	}
	return 0;
}
static  char * check_test() {
	assert(gs.initialized);
	printf("Running check_test\n");
	char fen[] = "rnbqk1nr/pppp1Bpp/8/2b1p3/4P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 1";
	//black king is in check from white bishop
	parseFen(fen);

	assert( isInCheck(BLACK));
	assert( !isInCheck(WHITE));
	return 0;
}

void testAux(char fen[], int num) {
	parseFen(fen);
	int numMoves;
	int movelist[200];
	getLegalMoveList( movelist, &numMoves);
	printf("number of moves = %d\n", numMoves);
	assert(num==numMoves);

}
static void test_get_legal_moves() {
	assert(gs.initialized);

	//char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	//testAux(fen, 48);

	char fen4[]= "8/3K4/2p5/p2b2r1/5k2/8/8/1q6 b - - 1 67";
	testAux(fen4, 50);

	char pos5[]="r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
	testAux(pos5, 46);

	char fen[]="6k1/1R3p2/4p1p1/p1r3Pp/8/2PR3P/r4P2/5K2 b - - 0 44";
	testAux(fen, 27);

	char fen3[]="8/5k1p/5Pp1/1p1pr3/2r4P/Pp2PB2/1Kb1P3/2R4R w - - 1 43";
	testAux(fen3, 22);

	char fen5[]="3r2k1/5pp1/1p2p3/p1nrP2p/P1p5/5N1P/1PP2PP1/R3R1K1 b - - 3 25";
	testAux(fen3, 31);
}

void illegal_move_test() {    // make sure it picks up illegal moves
	/*	assert(gs.initialized);
		printf("illegal_move_test\n");
		char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
		parseFen(fen);

		int move = createMoveFromString("f3-a8", WQ, BR, captureNoPromotion);
		assert(!(isMoveLegal(move)));
		printf("finished illegal_move_test\n");
	 */
}

static  void execute(char *answer) {

	U64 start = currentTimeMillisecs() ;
	search_debug=true;

	char moveStr[6];

	calcBestMove( moveStr);
	//printf("move found, answer ***%s***%s\n", moveStr, answer);
	U64 end =  currentTimeMillisecs() ;
	U64 duration = end-start;
	total=total+duration;
	int result = strcmp(answer,moveStr);
	if (result != 0 ) {
		printf("FAILED*****  to find the right move\n");
	}
	//total=total+duration;
	printf("Time elapsed %lld milliSeconds \n", duration);
	printf("New Move %s\n" , moveStr);

}

void run_search( char *fen, char *answer) {
	assert(gs.initialized);
	//printf("parseFen\n");
	parseFen(fen);
	//printf("execute\n");
	execute(answer);
	//printf("Finished execute\n");
}
static void winAtChess()
{

	char * fen[] = {

			"2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - - bm",
			"rnbqkbnr/pppp1ppp/4p3/8/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2",
			"rnbqkbnr/ppppp2p/8/5pp1/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq g6 0 3",
			"r4kr1/1b2R1n1/pq4p1/4Q3/1p4P1/5P2/PPP4P/1K2R3 w - - 0 1",
			"r3rk2/1p1n1p1Q/p1b1p3/2q3pN/8/2PB4/P1P2PP1/2KRR3 w - - 0 1",
			"8/8/8/8/8/7K/5k2/4R3 b - - 0 68",
			"5k2/6pp/p1qN4/1p1p4/3P4/2PKP2Q/PP3r2/3R4 b - - bm",   //wac005
			"2br2k1/2q3rn/p2NppQ1/2p1P3/Pp5R/4P3/1P3PPP/3R2K1 w - - bm", //wac010
			"r4q1k/p2bR1rp/2p2Q1N/5p2/5p2/2P5/PP3PPP/R5K1 w - - bm",
			"3qrbk1/ppp1r2n/3pP2p/3P4/2P4P/1P3Q2/PB6/R4R1K w - - bm",
			"6r1/3Pn1qk/p1p1P1rp/2Q2p2/2P5/1P4P1/P3R2P/5RK1 b - - bm"
	};
	char* answers[] = {
			"g3g6",
			"d8h4",
			"d1h5",
			"e7f7",
			"d3g6",
			"f2e1",
			"c6c4",
			"h4h7",
			"e7f7",
			"f3f7",
			"g6g3"
	};

	total=0;
	for (int i=0; i < 11; i++) {
		//printf("run search %s %s\n", fen[i], answers[i]);
		run_search( fen[i], answers[i]);
	}
	printf("Total %llu\n",  total);
}
static void checkTest() {
	printf("starting checkTest\n");
	char fen[]="rnbq1bnr/ppppkppp/4p3/8/8/BP6/P1PPPPPP/RN1QKBNR b KQkq - 1 1";
	parseFen(fen);

	bool isBlackInCheck = isInCheckAux( gs.bitboard[BK], WHITE);
	assert(isBlackInCheck);

	char p[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	parseFen(p);

	isBlackInCheck = isInCheckAux( gs.bitboard[BK], WHITE);
	assert(!isBlackInCheck);

	bool isWhiteInCheck = isInCheckAux( gs.bitboard[WK],  BLACK);
	assert(!isWhiteInCheck);


	// String s55 = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R4K1R b kq - 1 1";
	// gs = new GameState(s55);
	//isWhiteInCheck = gs.isInCheck( gs.bitboard[GameState.WK],GameState.BLACK);
	//assertEquals(false, isWhiteInCheck);


	// isWhiteInCheck = gs.isInCheck(GameState.WHITE);
	// assertEquals(false, isWhiteInCheck);
	printf("Finished checkTest\n");
}

void do_eval() {
	char fen[]="2rr3k/pp3pp1/1nNqbN1p/3p4/2pP3P/2P3Q1/PPB5/R4RK1 b - - 0 1";
	parseFen(fen);
	getEvaluation();
}

void repetitionTest() {
	if (!gs.initialized) return ;

	newGame();
	printf("Repetition Test\n");
	char moves[16][5] = { "b1c3","b8c6", "g1f3","g8f6",
					   "c3b1","c6b8", "f3g1","f6g8",
					   "b1c3","b8c6", "g1f3","g8f6",
					   "c3b1","c6b8", "f3g1","f6g8"  };

	for (int i=0; i< 16; i++) {
		char * move = moves[i];
		//printf("next move: %s\n", move);
		//printf("old hash %llu\n", gs.hash);
		applyMove(move);
		//printf("new hash %llu\n", gs.hash);
		//printf("testing move %d = %s\n", i, move);
		assert(i < 7 || isRepetition());
		/*
		if (isRepetition()) {
			printf("Repetition detected on move %d\n", i);
		}*/
	}

}
void bookTest() {
		printf("book test\n");
		char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		parseFen(fen);
		U64 hash = gs.hash;

		printf("first hash = %llu\n", hash);
		char fen2[70];


		toFEN(fen2);
		printf("fen=%s \n",fen2);
		parseFen(fen2);
		U64 hash2 = gs.hash;

		printf("second hash = %llu\n", hash2);
		assert(hash==hash2);
	   // printBookMoves();
		char moveStr[5]="1234";

		getBookMove(hash, moveStr);
		//printf("The book move is : %s\n", moveStr);
		assert(
				(strcmp(moveStr,"e2e4")== 0 ) ||
				(strcmp(moveStr,"d2d4")== 0 ) ||
				(strcmp(moveStr,"c2c4")== 0 ) ||
				(strcmp(moveStr,"g1f3")== 0 ) ||
				(strcmp(moveStr,"g2g3")== 0 ));
}
void do_all_tests(void) {

	initializeAll();
	printf("Finished initializing\n");
	checkTest();
	illegal_move_test();
	test_get_legal_moves();
	/* make_test_suite();
	see_test_suite();

	check_test();
	isLegal_test();
	eval_test();

	run_perft_test();
    check_evasion_test();
    hash_test();
	winAtChess();
	do_eval();

	bookTest();
	repetitionTest();*/
	printf("all tests successfully completed\n");
}

