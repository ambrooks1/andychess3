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
void evaluateA() {    //exchange pawns, should be +1
	int side=0;
	int victim=BP;

	int attackers[][16] = {  { WP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };

	int ptr[2];
	ptr[WHITE] = 0;
	ptr[BLACK] = 0;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(1 == result);

}
void evaluateB() {    // pawn takes knight defended by pawn, should be +225
	int side=0;
	int victim=BN;

	int attackers[][16] = {  { WP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };

	int ptr[2];
	ptr[WHITE] = 0;
	ptr[BLACK] = 0;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(225 == result);

}
void evaluateC() {    // queen takes knight defended by pawn, should be -650, which is 0
	int side=0;
	int victim=BN;

	int attackers[][16] = {  { WQ, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };
	int ptr[2];
	ptr[WHITE] = 0;
	ptr[BLACK] = 0;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(0 == result);

}
void evaluateD() {    // Rxr,  pxR, Rxp  = +100
	int side=0;
	int victim=BR;

	int attackers[][16] = {  { WR, WR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };

	int ptr[2];
	ptr[WHITE] = 1;
	ptr[BLACK] = 0;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(100 == result);

}
void evaluateE() {    // Rxr,  rxR, Rxr  =  500 ; white is a rook up
	int side=0;
	int victim=BR;

	int attackers[][16] = {  { WR, WR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };


	int ptr[2];
	ptr[WHITE] = 1;
	ptr[BLACK] = 0;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(500 == result);

}
void evaluateF() {    // Rxr,  rxR, Rxr, pxR ; even trade = 1
	int side=0;
	int victim=BR;

	int attackers[][16] = {  { WR, WR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BP, BR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };


	int ptr[2];
	ptr[WHITE] = 1;
	ptr[BLACK] = 1;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(1 == result);

}
void evaluateG() {    // Rxr,  pxR, Rxp, rxR ;   down -400 so = 0
	int side=0;
	int victim=BR;

	int attackers[][16] = {  { WR, WR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BR, BP, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };


	int ptr[2];
	ptr[WHITE] = 1;
	ptr[BLACK] = 1;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(0 == result);

}
void evaluateH() {    // rxN, RxR so should be 0
	int side=1;
	int victim=WN;

	int attackers[][16] = {  { WQ, WR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
			{ BR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }  };

	int ptr[2];
	ptr[WHITE] = 1;
	ptr[BLACK] = 0;

	int result = evaluate2(side, victim, attackers, ptr);
	printf("Result= %d\n", result);
	assert(0 == result);

}
void see_test_suite()  {
	evaluate();
	evaluateA();
	evaluateB();
	evaluateC();
	evaluateD();
	evaluateE();
	evaluateF();
	evaluateG();
	evaluateH();

}
