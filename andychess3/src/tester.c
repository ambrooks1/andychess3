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

extern gameState gs;
extern bool search_debug;

void run_search( char *fen, char *answer) ;
static const int depthLevel=7;

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
	int * moves = generateNonCaptures(gs.color, &numMoves);

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
		int * moves = getAllMoves(gs.color, &numMoves);
		printf("number of moves = %d\n", numMoves);
		assert(num==numMoves);
		free(moves);
}
static void test_get_all_moves() {
	assert(gs.initialized);

	char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	testAux(fen, 48);

	char fen4[]= "8/3K4/2p5/p2b2r1/5k2/8/8/1q6 b - - 1 67";
	testAux(fen4, 50);

	char pos5[]="r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
	testAux(pos5, 46);
}

static  char *  make_test() {
	assert(gs.initialized);
	printf("Running make_test\n");
	char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	parseFen(fen);
	int numMoves;
	int * moves = generateNonCaptures(gs.color, &numMoves);

	int saveBoard[64];
	memcpy(saveBoard, gs.board, sizeof(gs.board));

	U64 saveBitboards[NUMBITBOARDS];
	memcpy(saveBitboards, gs.bitboard, sizeof(gs.bitboard));

	int flags = gs.flags;
	U64 hash2 = gs.hash;

	for (int i=0; i < numMoves; i++)
	{
		int move = moves[i];
		//printf("doing make/unmake on move  %s\n",  moveToString(move));
		make(move);
		unmake(move, flags, hash2);

		int comp_val=memcmp(saveBoard, gs.board,  64*sizeof(int));
		assert(comp_val==0);

		comp_val=memcmp(saveBitboards, gs.bitboard,  NUMBITBOARDS*sizeof(U64));
		assert( comp_val==0);

		// TODO : compare material positional color flags and hash
	}
	return 0;
}

void illegal_move_test() {    // make sure it picks up illegal moves
		printf("illegal_move_test\n");
		char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
		parseFen(fen);

		int move = createMoveFromString("f3-a8", WQ, BR, captureNoPromotion);
		assert(!(isMoveLegal(move)));
		printf("finished illegal_move_test\n");

	}

static  char * execute() {
		 U64 start = currentTimeMillisecs() ;
		 search_debug=true;
		 char* moveStr = calcBestMove( gs, depthLevel);
		 U64 end =  currentTimeMillisecs() ;
		 U64 duration = end-start;
		 //total=total+duration;
		 printf("Time elapsed %lld milliSeconds \n", duration);
		 printf("New Move %s\n" , moveStr);
		 return moveStr;
	}

void run_search( char *fen, char *answer) {
	printf("parseFen\n");
	parseFen(fen);
	printf("execute\n");
	char * result = execute();
	printf("Finished execute\n");
	assert( strcmp(answer,result)==0);
	free(result);
}
static void winAtChess()
		{

		char * fen[] = {
				//"r1b1kb1r/pp2p1pp/3q4/5p2/3nQ3/P5NP/1PP2PP1/R1BBK2R w KQkq f6 0 16",
				//"r1bqkbnr/pppp1ppp/8/3Pn3/5P2/8/PPP3PP/RNBQKBNR b KQkq f3 0 5",
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
				//"3q1rk1/p4pp1/2pb3p/3p4/6Pr/1PNQ4/P1PB1PP1/4RRK1 b - - bm"
		};
		char * answers[] = {
				//"e4d3",
				//"e5g6",
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
				//"d6h2"
		};

		for (int i=0; i < sizeof(fen); i++) {
			printf("run search %s %s\n", fen[i], answers[i]);
			run_search( fen[i], answers[i]);
		}
		//System.out.println("Total " + total);
	}
int main(void) {

	initializeAll();
	printf("Finished initializing\n");
	//illegal_move_test();
	/*test_get_all_moves();
	make_test();
	check_test();
	isLegal_test();
	fflush(stdout);
	eval_test();*/

	//run_perft_test();

	winAtChess();
	return EXIT_SUCCESS;
}

