/*
 * TranspositionTable.c
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 *      based on sample code from Harm Geert Muller
 */

/*
 * typedef  struct transpositionTable {

	U64 hashValue;
	int bestmove;
	int depthValue;
	char flags;
	int nodeScore;

}
 */
#include <stdlib.h>
#include "defs.h"

transpositionTable tt[TRANS_TBL_ENTRIES];

const U64 hashMask = C64(0x7fffffffffffffff);
const int LOWERBOUND=0;
const int UPPERBOUND=1;
bool useTT;

int probes=0, hits=0, stores=0, usefulScores=0, repetitionsFound=0;

int tt_getIndex(U64 hash);

int tt_getDepth(U64 hash2) {
	int index2=tt_getIndex(hash2);
	return tt[index2].depthValue;
}
int tt_getScore(U64 hash2) {
	int index2=tt_getIndex(hash2);
	return tt[index2].nodeScore;
}
int tt_getMove(U64 hash2) {
	int index2=tt_getIndex(hash2);
	return tt[index2].bestmove;
}
int tt_getIndex(U64 hash)
{
	// mask to make sure it is a positive value
	hash &= hashMask;
	return (int)(abs(hash) % TRANS_TBL_ENTRIES);

}
bool  tt_probeHash(U64 hash2)
{
	int index2=tt_getIndex(hash2);
	probes++;

	if (tt[index2].hashValue == hash2 ) {
		hits++;
		return true;
	}
	return false;
}

void tt_hashStore(U64 hash2, int depth2, int move,  int score, int alpha, int beta )
{
	char boundType = 0;

	int index2=tt_getIndex(hash2);


	if  ( (tt[index2].depthValue > 0) &&
			(tt[index2].depthValue < depth2) && ( hash2== tt[index2].hashValue)) {
		// don't overwrite if the bestmove is already in the table  resulting from a deeper search
		return;
	}
	stores++;
	tt[index2].hashValue = hash2;
	tt[index2].bestmove=move;
	tt[index2].depthValue=depth2;
	tt[index2].nodeScore=score;

	if(score > alpha) boundType |= LOWERBOUND; // if >= beta the true score could be higher, because we did ignore moves after cutoff
	if(score < beta)  boundType |= UPPERBOUND; // if <= alpha true score could be lower, because the daughters had beta cutoffs
	tt[index2].flags = boundType;                   // note that scores between alpha and beta are exact, so both upper and lower bound
}
bool tt_usefulScore(U64 hash2, int alpha, int beta)
{
	int index2=tt_getIndex(hash2);
	int score2 = tt[index2].nodeScore;
	int x = tt[index2].flags & LOWERBOUND;
	int y = tt[index2].flags & UPPERBOUND;

	// scores above alpha must be lower bounds, or the current alpha is lower than the one used for calculating the score
	if( score2 > alpha && ( x== 0)) return false;
	// scores below beta must be upper bounds, or the current beta is larger than the one used for calculating the score
	if( score2 < beta  && ( y == 0 )) return false;
	usefulScores++;
	return true; // if the score was exact, true upper bound <= alpha or true lower bound >= beta, it is meaningful
}
