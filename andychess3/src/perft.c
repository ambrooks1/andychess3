/*
 * perft.c

 *
 *  Created on: Mar 6, 2014
 *      Author: andrewbrooks
 */



/**
 * This class is used to conduct a performance test on the move generation
 * process.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "defs.h"
#include "gamestate.h"
#include "timecontrol.h"
#include "move.h"

U64 nodes;
U64 captures;
U64 checks;
U64 enPassant;
U64 castles;
U64 promotions;
gameState state;
static bool debug=true;

void crawl(gameState state, int depth);

void perft_test(int depth)
{

	nodes = 0;
	checks = 0;
	captures = 0;
	enPassant = 0;
	castles = 0;
	promotions = 0;

	U64 start = currentTimeMillisecs();

	crawl(state, depth);

	U64 end = currentTimeMillisecs();

	if (debug) printf("nodes=  %lld, captures= %lld ep= %lld , castles= %lld , promotions= %lld checks %lld\n", nodes, captures, enPassant,
			castles, promotions , checks);

	float time = (float)(end-start)/1000;

	if (debug)  printf("time= %.2f nps %.2f\n", time, (float)nodes/time);
}

void crawl(gameState state, int depth)
{
	if(depth == 0)
	{
		return;
	}
	else
	{

		int numMoves;
		int * moves = getAllMoves(state.color, &numMoves);
		int flags=state.flags;
		U64 hash=state.hash;

		for (int i=0; i < numMoves; i++)
		{
			int move = moves[i];
			make(move);

			if(depth == 1)
			{
				nodes++;
				int type =moveType(move);
				switch(type) {
				case kcastle: case qcastle :

				castles++;
				break;
				case captureNoPromotion:
				captures++;
				break;

				case capturePromotionQueen:
				captures++;
				promotions++;
				break;

				case simplePromotionQueen:
				promotions++;
				break;
				case epCapture:
				enPassant++;
				captures++;
				}

				if(isInCheck( state.color))
					checks++;
			}
			crawl(state,depth-1);
			unmake(move,flags,hash);
		}
	}
}

U64 getNodes() {
	return nodes;
}

void setNodes(U64 nodes2) {
	nodes = nodes2;
}

U64 getCaptures() {
	return captures;
}

void setCaptures(U64 captures2) {
	captures = captures2;
}

U64 getChecks() {
	return checks;
}

void setChecks(U64 checks2) {
	checks = checks2;
}

U64 getEnPassant() {
	return enPassant;
}

void setEnPassant(U64 enPassant2) {
	enPassant = enPassant2;
}

U64 getCastles() {
	return castles;
}

void setCastles(U64 castles2) {
	castles = castles2;
}

U64 getPromotions() {
	return promotions;
}

void setPromotions(U64 promotions2) {
	promotions = promotions2;
}

gameState getState() {
	return state;
}

void setState(gameState state2) {
	state = state2;
}


