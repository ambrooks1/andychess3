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
extern gameState gs;

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

int main(void) {

	initializeAll();
	printf("Finished initializing\n");
	//illegal_move_test();
	test_get_all_moves();
	make_test();
	check_test();
	isLegal_test();
	fflush(stdout);
	eval_test();

	//run_perft_test();


	return EXIT_SUCCESS;
}

