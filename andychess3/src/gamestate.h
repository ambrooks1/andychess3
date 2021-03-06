/*
 * andychess3.h
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */

#ifndef ANDYCHESS3_H_
#define ANDYCHESS3_H_

#include <stdbool.h>


bool isBitSet(int x, int position);
int getEPSquare();
void parseFen2 ( char fen[]);
void printBoard();
void initializeAll();
void parseFen( char fen[]);
void unmake(MOVE move, int flags2, U64 hash2);
void make(MOVE move);
void generateCapturesAndPromotions(int color, MOVE moves[], int* cntMoves, U64 bitboard[], int board[] );
void generateNonCaptures(int color, MOVE moves[], int* numMoves, U64 bitboard[] );
int materialValue(int color) ;
int addPositionalValueForType ( int piece);
U64 calcHash();
int getPSTValue2(int victim, int to);
bool isInCheckAux( U64 king, int color) ;
bool isSameSquareRecapture() ;
bool isEndGame() ;
bool isMoveLegal ( MOVE move) ;
bool isInCheck( int color);
bool isInCheckAux( U64 king, int color);
void getAllMoves( int color, MOVE moves[], int* cntMoves);
#endif /* ANDYCHESS3_H_ */
