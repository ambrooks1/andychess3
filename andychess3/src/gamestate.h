/*
 * andychess3.h
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */

#ifndef ANDYCHESS3_H_
#define ANDYCHESS3_H_

#include <stdbool.h>

void printBoard();
void initializeAll();
void parseFen( char fen[]);
void unmake(int move, int flags2, U64 hash2);
void make(int move);
int* generateCapturesAndPromotions(int color, int *numMoves ) ;
int* generateNonCaptures(int color, int *numMoves  );
int materialValue(int color) ;
int addPositionalValueForType ( int piece);
U64 calcHash();
int getPSTValue2(int victim, int to);
bool isInCheckAux( U64 king, int color) ;
bool isSameSquareRecapture() ;
bool isEndGame() ;
bool isMoveLegal ( int move) ;
bool isInCheck( int color);
bool isInCheckAux( U64 king, int color);
int* getAllMoves( int color, int* cntMoves);
#endif /* ANDYCHESS3_H_ */
