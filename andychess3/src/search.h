/*
 * search.h
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */

#ifndef SEARCH_H_
#define SEARCH_H_

#define MAX_DEPTH 200

#define MOVEGEN_CHECK_EVASION_PHASE  	4
#define MOVEGEN_HASHMOVE2_PHASE  		3
#define MOVEGEN_NONCAPTURES_PHASE  		2
#define MOVEGEN_CAPTURES_PHASE  		1
#define MOVEGEN_HASHMOVE_PHASE  		0

#define INVALID_MOVE 					-1
#define TIME_CHECK_INTERVAL 1000; // How often we should check if time is up inside alphaBeta
#define LEVEL_INCREASE_TIME_MULTIPLE  2.0;

#define CAPTURE_SORT_VALS 				32

#define  MAX_INFINITY  					500000
#define  MIN_INFINITY  					-500000

#define   R  							2     // used for null move
#define  FullDepthMoves  				4
#define   ReductionLimit  				3
#define   MAXDEPTH 						20

#define  NO_HISTORY_SORT_VAL  			63
#define   HISTORY_POINTS  				30

	//put it in the end if no history - if we do see we should change this to 62, because bad captures get 63

#define   NUMKILLERS 					2
#define  MATE 							100000
#define MAX_MOVES						200
#define LOGGING_STRING_SIZE				1000

int quies( gameState state, int alpha, int beta, int depth);
bool isLegal (gameState state,const int move, int flags2, U64 hash, int moveType) ;
int getNextMove(int i, int* moves, int numMoves);
int lmr_search(gameState state, int alpha, int beta,
		int depth, int mate, bool extended, int legal,
		bool opponentIsInCheck, int searchDepth, int myOrderingValue) ;
int calcBestMoveAux(gameState state, int depthlevel, int alpha, int beta);
void updateKillerAndHistory(gameState state, int depth, int move) ;
void orderMoves(gameState state, int* movelist, int numMoves, int depth, int hashMove, bool hashFound);
void orderMovesCaps(gameState state, int* movelist, int numMoves,  U64 hash, int depth, int hashMove, bool hashFound);
void printLoggingInfo(int currentDepth, int bestMove, int score);
int pv_search(gameState state, int alpha, int beta,
		int mate, bool extended, bool foundPv, int searchDepth) ;
bool isCapture(int myMoveType) ;
int search(gameState state, int alpha, int beta,
		int depth, int mate, bool allowNull, bool extended, bool returnBestMove);
void  orderCapturesBySee(gameState state, int* movelist, int numMoves) ;

#endif /* SEARCH_H_ */
