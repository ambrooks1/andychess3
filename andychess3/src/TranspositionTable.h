/*
 * TranspositionTable.h
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */

#ifndef TRANSPOSITIONTABLE_H_
#define TRANSPOSITIONTABLE_H_

#include <stdbool.h>
int tt_getDepth(U64 hash2);
int tt_getScore(U64 hash2);
bool  tt_probeHash(U64 hash2);
int tt_getIndex(U64 hash);
int tt_getMove(U64 hash2) ;
bool tt_usefulScore(U64 hash2, int alpha, int beta);
void tt_hashStore(U64 hash2, int depth2, int move,  int score, int alpha, int beta );
#endif /* TRANSPOSITIONTABLE_H_ */
