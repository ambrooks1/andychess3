/*
 * andychess3.h
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */

#ifndef ANDYCHESS3_H_
#define ANDYCHESS3_H_

#include <stdbool.h>

int* generateCapturesAndPromotions(int color ) ;
int* generateNonCaptures(int color );
int materialValue(int color) ;
int addPositionalValueForType ( int piece);
U64 calcHash();
int getPSTValue2(int victim, int to);
bool isInCheckAux( U64 king, int color) ;

#endif /* ANDYCHESS3_H_ */
