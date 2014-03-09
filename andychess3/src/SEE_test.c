/*
 * SEE_test.c
 *
 *  Created on: Mar 9, 2014
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
#include "SEE.h"
#include "eval.h"
#include "gamestate.h"

void evaluate() {
		printf("SEE test : evaluate\n");
	    int side=0;
		int victim=BP;

		int attackers[][16] = {
				{WP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
				{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };

		int ptr[2];
		ptr[WHITE] = 0;
		ptr[BLACK] = -1;

		int result = evaluate2(side, victim, attackers, ptr);
		printf("Result= %d\n", result);
		assert(100 == result);
	}

void see_test_suite()  {
	evaluate();
}
