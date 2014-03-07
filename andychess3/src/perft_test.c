/*
 * perft_test.c
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
#include "perft.h"
#include <assert.h>
#include "timecontrol.h"

//extern gameState gs;

U64 nodes[]= { 0, 20, 400, 8902, 197281, 4865609, 119060324};
U64 captures[]= { 0, 0, 0, 34, 1576, 82719, 2812008};
U64 ep[]= { 0, 0, 0, 0, 0, 258, 5248};
U64 checks[]= { 0, 0, 0, 12, 469, 27351, 809099};
void run_perft_test()
{

	char startPos[] =
			"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	parseFen(startPos);

	for (int i=1; i <= 5; i++) {
		//perft_test(i);
		U64 start = currentTimeMillisecs();
		int nodes2 = perft2(i);
		U64 end = currentTimeMillisecs();
		printf("nodes %d\n", nodes2);
		float time2 = (float)(end-start)/1000;
		float fnodes = (float)nodes2;
		float nps = fnodes/time2;
	    printf("time= %.2f nps %.2f\n", time2, nps);

		assert(nodes[i]== nodes2);
		//assert(captures[i]== getCaptures());
		//assert(ep[i]== getEnPassant());
		//assert(checks[i]== getChecks());
	}
}
