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
		perft_test(i);
		assert(nodes[i]== getNodes());
		assert(captures[i]== getCaptures());
		assert(ep[i]== getEnPassant());
		assert(checks[i]== getChecks());
	}
}
