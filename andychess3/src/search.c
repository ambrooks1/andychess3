/*
 * search.c

 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include "assert.h"
#include "defs.h"
#include "search.h"
#include "timecontrol.h"
#include "search.h"
#include "move.h"
#include "gamestate.h"
#include "SEE.h"
#include "TranspositionTable.h"
#include "eval.h"
#include "movegen.h"
#include "util.h"


int  nextTimeCheck = TIME_CHECK_INTERVAL;
int movesPerSession=0;   //default
int movesMade=0;
U64 baseTime=0;     // in milliseconds; default = 15 minutes
U64 increment=0;

U64 timeLeft=0;     //time left for the player
U64 opponentTimeLeft=0;     // time left for the opponent
U64 timeForThisMove;
bool  stopSearch = true;
U64 startTime=0;
int depthLevel=0;

int valWINDOW=35;
extern bool useTT;
extern int probes, hits, stores;
extern gameState gs;
extern const U64 minedBitboards[2][2];
extern int numBookMovesMade;
extern const int valueMap[];

int         numCutoffs=0,
		firstMoveCutoffs=0,
		nodesSearched=0,
		numQuiesNodes=0,
		usingHashMove=0,
		usingKillerMove=0,
		futilityPrune=0;

int history[2][64][64];

int killer[MAXDEPTH][NUMKILLERS];
bool        useIID,do_LMR,
extensionsOn, futilityOn,aspirationOn,
turnNullOn,deltaPruneOn,positionalEvalOn,orderByHistory,
orderByKillers,turnSEEOn;

bool foundMove=false;
bool search_debug=false;
bool usingTime=false;

void clearHistory() {
	for (int i=0 ; i < 2; i++) {
		for (int j=0; j < 64; j++) {
			for (int k=0; k < 64; k++) {
				history[i][j][k]=0;
			}
		}
	}
}

void turnEverythingOn() {
	useIID=true;
	useTT=true;
	do_LMR=true;
	extensionsOn=true;
	futilityOn=true;
	aspirationOn=true;
	turnNullOn=true;
	positionalEvalOn=true;
	deltaPruneOn=true;
	orderByKillers=true;
	orderByHistory=true;
	turnSEEOn=false;
}


void zeroOutCounters() {
	numCutoffs=0;
	firstMoveCutoffs=0;
	nodesSearched=0;
	numQuiesNodes=0;
	stopSearch=false;
	usingHashMove=0;
	usingKillerMove=0;
	futilityPrune=0;
	stores=0;

	srand(time(NULL));
	int x = rand();

	if (x % 4 ==0 ) clearHistory();
	for (int i=0; i < MAXDEPTH; i++) {
		for (int j=0; j < NUMKILLERS; j++) {
			killer[i][j]=0;
		};
	}
}

void initGlobals() {
	zeroOutCounters();
	turnEverythingOn();
	gs.moveCounter=0;
}

void calcBestMove( int depthLevel2, char *moveStr) {
	depthLevel=depthLevel2;
	initGlobals();

	int move = calcBestMoveAux(depthLevel, MIN_INFINITY, MAX_INFINITY);
	movesMade++;
	moveToString(move, moveStr);
}

void sort_by_value(MoveInfo arr[], int size) {
	// note the size-1.  The last one will be in order
	for (int i = 0; i < size-1; i++) {
		int min = i;
		for (int j = i+1; j < size; j++) {
			if (arr[j].value < arr[min].value) {
				min = j;
			}
		}
		if (min != i) { // if you're not changing position, don't swap
			MoveInfo temp = arr[i];
			arr[i] = arr[min];
			arr[min] = temp;
		}
	}
}
void sorter(int arr[], int size) {
	// note the size-1.  The last one will be in order
	for (int i = 0; i < size-1; i++) {
		int min = i;
		for (int j = i+1; j < size; j++) {
			int myOrderingValue =  orderingValue(arr[j]);
			int myOrderingValue2 = orderingValue(arr[min]);

			if (myOrderingValue < myOrderingValue2) {
				min = j;
			}
		}
		if (min != i) { // if you're not changing position, don't swap
			int temp = arr[i];
			arr[i] = arr[min];
			arr[min] = temp;
		}
	}
}
void makeMoveInfo(int movelist[], MoveInfo movelist2[],  int cntMoves) {
	for (int i=0; i < cntMoves; i++) {
		movelist2[i].move = movelist[i];
		movelist2[i].value=0;
	}
}

void printMovelist(MoveInfo mi[], int cntMoves) {
	printf("do it**********\n");
	char s[5];
	for (int i=0; i < cntMoves; i++)
	{
		moveToString(mi[i].move, s );
		printf("moveInfo move  %s\n",  s);
		printf("moveInfo  value %d\n",  mi[i].value );
	}
	printf("done***********\n");
}

void getTheMovelist(MoveInfo movelist2[], int *cntMoves2) {
	int movelist[200];
	bool ownKingInCheck=isInCheck( gs.color);

	int cntMoves;
	if (ownKingInCheck)
		generateCheckEvasionMoves(gs.color, movelist, &cntMoves);
	else
		getAllMoves(gs.color, movelist, &cntMoves);

	makeMoveInfo(movelist, movelist2, cntMoves);

	*cntMoves2=cntMoves;
}

int calcBestMoveAux( int depthlevel, int alpha, int beta)  {

	int    bestMove=0;
	int 	cntMoves;
	MoveInfo movelist[200];

	getTheMovelist(movelist, &cntMoves);
	//printMovelist(movelist, cntMoves);

	int flags2=gs.flags;
	U64 hash=gs.hash;

	int currentDepth=1;

	int maxIterations=depthLevel;
	if (usingTime) {
		maxIterations=MAXDEPTH;
	}

	if (usingTime) {
		startTime=currentTimeMillisecs();
		timeForThisMove = getTimeForThisMove(timeLeft,
				movesPerSession- ( ( movesMade + numBookMovesMade) % movesPerSession ), increment);
	}
	int bestScoreForThisIteration = 0;

	while (currentDepth < maxIterations) {
		// we don't have enough time to do this iteration, so return the global best move
		if (usingTime) {
			U64 elapsedTime = currentTimeMillisecs() - startTime;
			U64 adjustedTime= elapsedTime * LEVEL_INCREASE_TIME_MULTIPLE;
			if  (adjustedTime > timeForThisMove) {
				//if (debug) printLoggingInfo(currentDepth, bestMove, bestScoreForThisIteration);
				return bestMove;
			}
		}
		bestScoreForThisIteration= MAX_INFINITY;

		sort_by_value(movelist,cntMoves);

		//printMovelist(movelist2, cntMoves);

		for (int i=0; i < cntMoves; i++)
		{
			int move = movelist[i].move;

			int myMoveType = moveType(move);
			assert(myMoveType >=1 && myMoveType <= 9);
			if (!isLegal( move, flags2, hash, myMoveType)) continue;


			int searchDepth=currentDepth;
			int score;
			assert(searchDepth < MAX_DEPTH);
			score = search( alpha, beta,  searchDepth,MATE,true,false, false);

			if (usingTime && stopSearch) {
				unmake(move, flags2, hash);
				break;
			}
			movelist[i].value=score;
			unmake(move, flags2, hash);

		/*	if (search_debug) {
				char s[5];
				moveToString(move,s);
				printf("Level  %d  move %s value %d bestval %d alpha %d beta %d\n",
						currentDepth, s,
						score,
						bestScoreForThisIteration,alpha, beta);
			}*/

			if (score < bestScoreForThisIteration) {
				bestScoreForThisIteration=score;
				bestMove=move;

				if (score <= alpha) {
					alpha=score;
					if (score < beta) break;
				}
			}
		}// end processing all moves loop

		if (aspirationOn) {
			if (bestScoreForThisIteration <= alpha) {
				alpha = MIN_INFINITY;
				continue;
			}
			else
				if  (bestScoreForThisIteration >= beta) {
					beta = MAX_INFINITY;
					continue;
				}
			alpha = bestScoreForThisIteration - valWINDOW;
			beta = bestScoreForThisIteration + valWINDOW;
		}

		currentDepth++;
	}  //end of iteration deepening loop
	if (search_debug) printLoggingInfo(currentDepth, bestMove, bestScoreForThisIteration);
	//ponderMove=getPonderMove(gs, bestMove);

	return bestMove;
}
void printLoggingInfo(int currentDepth, int bestMove, int score) {
	char str[LOGGING_STRING_SIZE];
	char s[5];
	moveToString(bestMove,s);
	sprintf(str,
			"bestmove  %s score %d   currentDepth  %d time for this move %lld   Num regular nodes  = %d   numQuiesNodes %d\n",
			s, score,
			currentDepth, timeForThisMove, nodesSearched, numQuiesNodes);

	U64 totalNodes = nodesSearched + numQuiesNodes;

	float orderingSuccess = (float)firstMoveCutoffs/ (float)numCutoffs;

	char str2[100];
	sprintf(str2, "numCutoffs %d  firstMoveCutoffs %d",numCutoffs, firstMoveCutoffs);
	strcat(str, str2);

	sprintf(str2," total  nodes  %lld =  orderingSuccess %.2f\n", totalNodes, orderingSuccess);
	strcat(str, str2);

	sprintf(str2," probes  %d hits  %d Using hash values %d\n", probes, hits,usingHashMove);
	strcat(str, str2);

	sprintf(str2,"hashtable hit rate %.2f stores %d\n", (float)hits / (float)probes, stores );
	strcat(str, str2);

	sprintf(str2,"futility Prunes %d usingKillerMoves %d\n", futilityPrune, usingKillerMove);
	strcat(str, str2);

	printf("%s",str);
}
bool isLegal (const int move, int flags2, U64 hash, int moveType) {

	if (moveType == kcastle || moveType == qcastle) {    //castling
		if (isInCheck( gs.color)) {
			return false;
		}
		make(move);
		// you cannot make a move that puts yourself in check
		if (isInCheck(  1- gs.color)) {
			unmake(move, flags2, hash);

			return false;
		}
		if (isInCheck( 1-gs.color)) {
			unmake(move, flags2, hash);
			return false;
		}
		if (isInCheckAux(  minedBitboards[1-gs.color][moveType-2],  gs.color) )  {
			unmake(move, flags2, hash);
			return false;
		}
	}
	else {
		make(move);
		// you cannot make a move that puts yourself in check
		if (isInCheck(  1- gs.color)) {
			unmake(move, flags2, hash);

			return false;
		}
	}
	return true;
}

/*  ALPHA BETA   ( Comment by Harm Geert Muller )
 *
 * Some branches need not be searched, because whatever their score,
 *  one of the players will get a better one by deviating from it earlier.
This is expressed by the 'search window' {alpha, beta}:
in any node, alpha is the score of the best move for the current side to move that has
already been found in any of the nodes on the path leading to (and including) the current node.
Beta is the same for the opponent, but seen from the current mover's POV.
If a move scores below alpha, we are not interested how much below alpha it scores, as we are not going to play it,
because we already did have a fully verified move that scores alpha, and will prefer that.

If we have a move that scores above beta, it is not important how much above beta it scores,
as our score above beta for the opponent looks as a score below his alpha,
which is the situation described in the previous sentence:
he will now prefer the verified move that scores beta (from our POV),
so we will never get the chance to play the move that (for us) scored above it.

So we can stop searching alternative moves from a position as soon as we find one move
that scores above beta.

This move is then a refutation of the opponent's preceding move, which is apparently so bad he is never going to play it.
Trying to show it is even more bad is a waste of time, so we will abort the search of that node at this point.
This is called a beta cutoff.

This sounds pretty complex, but the program that does it is actually quite simple.
Every time we find a better move, we increase alpha.
This is passed to all daughter nodes, which, two ply later, will include their best moves in it (if they are even better), etc.

In the intermediate node this our alpha becomes the opponent's beta
(after the sign flip to effect the change in POV characteristic of negamax), and finally passed to the grand-children of the current node as alpha again:
 *
 */

void displayMoves(int movelist[], int numMoves) {
	for (int i=0; i < numMoves; i++) {
		int move = movelist[i];
		int sortval = orderingValue(move);
		char s[5];
		moveToString(move, s);
		printf("%s order %d\n", s, sortval);
	}
}
int search( int alpha, int beta,
		int depth, int mate, bool allowNull, bool extended, bool returnBestMove)
{
	assert(depth < MAX_DEPTH);
	if (usingTime) {
		nextTimeCheck--;
		if(nextTimeCheck == 0) // Time to check the time
		{
			nextTimeCheck = TIME_CHECK_INTERVAL;
			if((currentTimeMillisecs() - startTime) > timeForThisMove)
			{
				stopSearch = true;
				return 0;
			}
		}
	}
	U64 hash=gs.hash;
	int flags2=gs.flags;
	bool foundPv=false;

	bool ownKingInCheck=isInCheck( gs.color);
	if (depth == 0) {

		if ( (extensionsOn) && (!extended) && ( gs.promotion || gs.seventhRankExtension
				|| isSameSquareRecapture() || ownKingInCheck))
		{
			assert(depth < MAX_DEPTH);
			depth++;
			extended = true;
		}
		else{
			return quies(alpha,beta,depth);
		}
	}

	int bestScore = MIN_INFINITY;
	int bestMove = INVALID_MOVE;


	//null move; ( not allowed when in check )
	/****************************************************************
	 *   This comment ( but not the code) is from CPW- engine
	 *  null move pruning:  allowing   *
	 *  opponent  to  execute two moves in a row,  i.e.  capturing   *
	 *  something  and escaping a recapture. If this cannot  wreck   *
	 *  our  position,  then it is so good that there's  no  point   *
	 *  in searching further. The flag allowNull ensures we don't   *
	 *  do  two null moves in a row. Null move is not used in  the   *
	 *  endgame because of the risk of zugzwang.                     *
	 ****************************************************************/
	if  ( turnNullOn && (!ownKingInCheck) && allowNull  && ((depth - 1 - R) > 2) && (!isEndGame())) {
		gs.color = 1-gs.color;
		if ( gs.color==BLACK) gs.hash = gs.hash ^ gs.side;
		assert(depth < MAX_DEPTH);
		int nullMoveScore= -search( -beta, -beta + 1,depth - 1 - R,  mate-1, false,false,false);

		gs.color = 1-gs.color;
		if ( gs.color==BLACK) gs.hash = gs.hash ^ gs.side;

		if (nullMoveScore >= beta) {

			if (useTT) {
				tt_hashStore(gs.hash, depth, bestMove,bestScore, alpha, beta);
				// *** alpha and beta are needed to determine meaning of score (bound type)
			}
			return beta;
		}
	}
	/****************************************************************
	 * COMMENT from CPW engine
	 *  If  depth  is too low for a null move pruning,  decide  if   *
	 *  futility  pruning is  applicable. If we are not  in  check,  *
	 *  not searching for a checkmate and eval is well below alpha,  *
	 *  it  might  mean that searching non-tactical moves  at  low   *
	 *  depths is futile, so we set a flag allowing this pruning.    *
	 ****************************************************************/
	// Futility pruning
	bool fprune = false;
	int fmargin = 0;
	int materialEval = 0;

	if   ( (depth ==1 && !ownKingInCheck)){
		materialEval =getEvaluationMaterial(gs);

		if (materialEval + 200  <= alpha){
			fmargin = 200;
			fprune = true;
		}
	}
	if (useTT) {
		bool found = tt_probeHash(gs.hash);               // *** check if this position was visited before, and if we still have info about it

		if(found) {

			if(tt_usefulScore(hash, alpha, beta)) {       // *** upper/lower bound might be useless with current {alpha, beta}
				bestScore = tt_getScore(hash);

			}
			bestMove = tt_getMove(hash)  ;                  // *** use move anyway, for lack of a better alternative
		}
	}
	/*---------------------------------------------------------
			   | Internal iterative deepening: if this is a PV node and |
			   | there is no best move from the hash table, then do a   |
			   | shallow search to find a best move to search first.    |
			   ---------------------------------------------------------*/
	if ( (useIID ) && ( alpha != beta+1 ) && ( bestMove == INVALID_MOVE) && (depth > 3) && (allowNull) )  {
		//bestMove = search( alpha, beta,
		//		depth - 2, mate, allowNull, extended, true)   ;
	}
	int movelist[200];
	int legal=0;

	int childrenSearched=0;

	int numMoves=0;

	int movegen_phase= MOVEGEN_HASHMOVE_PHASE;
	int lastMovegen_phase=MOVEGEN_NONCAPTURES_PHASE;

	if (ownKingInCheck) {
		movegen_phase=MOVEGEN_HASHMOVE2_PHASE;
		lastMovegen_phase=MOVEGEN_CHECK_EVASION_PHASE;
	}

	bool hashFound=false;

	while ( movegen_phase <= lastMovegen_phase)
	{
		switch(movegen_phase) {

		case MOVEGEN_HASHMOVE_PHASE:	//normal
		case MOVEGEN_HASHMOVE2_PHASE:   // for check evasions
			if (bestMove != INVALID_MOVE) {
				if ( isMoveLegal(bestMove)) {
					movelist[0]=bestMove;
					numMoves=1;
					usingHashMove++;
					hashFound=true;
				}
				else {
					movegen_phase++;
					continue;
				}
			}
			else {
				movegen_phase++;
				continue;
			}
			break;

		case MOVEGEN_CAPTURES_PHASE:
			generateCapturesAndPromotions(gs.color, movelist, &numMoves);
			if (numMoves > 0 ) {
				assert(depth < MAX_DEPTH);
				orderMovesCaps( movelist, numMoves, hash,depth, bestMove, hashFound);
			}
			break;

		case MOVEGEN_NONCAPTURES_PHASE:
			generateNonCaptures(gs.color, movelist, &numMoves);
			if (numMoves > 0 ) {
				assert(depth < MAX_DEPTH);
				orderMoves( movelist, numMoves, depth, bestMove, hashFound);
			}
			break;
		case MOVEGEN_CHECK_EVASION_PHASE:
			//generateCheckEvasionMoves(gs.color, movelist, &numMoves);
			getAllMoves(gs.color, movelist, &numMoves);
			if (numMoves > 0 ) {
				if (depth >= MAX_DEPTH) {
					printf("OUCH depth == %d\n", depth);
				}
				assert(depth < MAX_DEPTH);
				orderMoves( movelist, numMoves, depth, bestMove, hashFound);
			}
			break;
		}
		//displayMoves(movelist, numMoves);
		for (int i=0; i < numMoves; i++)
		{
			int move = getNextMove(i, movelist, numMoves) ;
			if (move==0) {
				continue;
			}
			int myMoveType = moveType(move);

			if (!isLegal( move, flags2, hash, myMoveType)) {
				continue;
			}

			legal++;
			bool opponentIsInCheck = isInCheck(gs.color);

			// Futility pruning,
			/********************************************************************
			 *  When the futility pruning flag is set, prune moves which do not  *
			 *  give  check and do not change material balance.
			 ********************************************************************/

			if (futilityOn && fprune && (childrenSearched > 0)) {
				if (!opponentIsInCheck) {
					if (isCapture(move)) {
						materialEval += valueMap[capture(move)];
					}
					if((materialEval+fmargin) <= alpha) {
						futilityPrune++;
						unmake(move, flags2, hash);
						continue;
					}
				}
			}

			int score;
			int searchDepth=depth-1;

			//late move reduction ? Here is where the idea is from: http://www.glaurungchess.com/lmr.html
			int myOrderingValue=orderingValue(move);

			if (do_LMR) {
				score = lmr_search( alpha, beta, depth, mate,
						extended, legal, opponentIsInCheck, searchDepth,
						myOrderingValue);
			}
			else {
				score = pv_search( alpha, beta, mate, extended,
						foundPv, searchDepth);
			}

			nodesSearched++;
			childrenSearched++;

			unmake(move, flags2, hash);

			if(score > bestScore)
			{                  // *** score accounting: remember best (from side-to-move POV)
				bestScore = score;

				if(score > alpha) {                    // *** narrow search window for remaining moves
					alpha = score;
					bestMove = move;
					foundPv = true;

					if(score >= beta)  {
						if (legal==1) firstMoveCutoffs++;
						numCutoffs++;

						if ( myMoveType < 10 )   // not a capture or promotion
						{
							updateKillerAndHistory( depth, move);
						}
						goto done;             // *** beta cutoff: previous ply is refuted by this move, so we can stop here to take that back
					}
				}
			}
		} // next move
		movegen_phase++;
	}  // next phse

	done:
	if (legal == 0) {
		if (! isInCheck( gs.color) )  {
			return 0;
		}
		return -mate;                 // Checkmate.
	}
	if (useTT) {
		tt_hashStore(hash, depth, bestMove,bestScore, alpha, beta);
		// *** alpha and beta are needed to determine meaning of score (bound type)
	}
	if (returnBestMove)
		return bestMove;
	return bestScore;
}
int pv_search( int alpha, int beta,
		int mate, bool extended, bool foundPv, int searchDepth) {
	int score;
	assert(searchDepth < MAX_DEPTH);
	if (foundPv) {
		score = -search(   -alpha - 1, -alpha, searchDepth,mate - 1, true,extended, false);   // reduced window pv search
		if ((score > alpha) && (score < beta)) // Check for failure.
			score = -search(  -beta, -alpha,searchDepth, mate - 1, true,extended, false);   // do regular search
	}
	else {
		score = -search(  -beta, -alpha,searchDepth, mate - 1, true,extended, false);   // do regular search
	}
	return score;
}
int lmr_search( int alpha, int beta,
		int depth, int mate, bool extended, int legal,
		bool opponentIsInCheck, int searchDepth, int myOrderingValue) {
	int score;
	assert(searchDepth < MAX_DEPTH);
	if (legal == 1) {
		score = -search( -beta, -alpha,searchDepth, mate - 1, true,extended, false);        //normal alpha-beta search
	}
	else
	{
		if ( (!extended) && (legal >= FullDepthMoves) && (depth >= ReductionLimit)
				&& (myOrderingValue >= CAPTURE_SORT_VALS + 1)
				&& (!opponentIsInCheck) && ( beta - alpha <= 1))
		{
			score = -search(   -alpha - 1, -alpha, searchDepth-1,mate - 1, true,extended, false);   // reduced window pv search
		}
		else
		{
			score = alpha+1;  // Hack to ensure that full-depth search is done.
		}
		if(score > alpha)
		{
			score = -search(   -alpha - 1, -alpha, searchDepth,mate - 1, true,extended, false);   // reduced window pv search
			if ((score > alpha) && (score < beta)) // Check for failure.
				score = -search(  -beta, -alpha,searchDepth, mate - 1, true,extended, false);   // do regular search
		}
	}
	return score;
}

void updateKillerAndHistory( int depth, int move) {
	int f = fromIndex(move);
	int t = toIndex(move);

	if (history[gs.color][f][t] < HISTORY_POINTS)
		history[gs.color][f][t] ++;

	int killerValue=killer[depth][0];

	if (killerValue == 0 ) {
		killer[depth][0]=move;
	}
	else {
		killer[depth][1]=killerValue;
		killer[depth][0]=move;
	}
}
/***************************************************************
 *  At leaf nodes we do quiescence search                       *
 *  to make sure that only relatively quiet positions           *
 *  with no hanging pieces will be evaluated.                   *
 ***************************************************************/
int quies(  int alpha, int beta, int depth)
{

	int val ;

	if (positionalEvalOn)
		val= getEvaluation();
	else
		val=getEvaluationMaterial();

	if (val >= beta) {
		return beta;
	}
	if (val > alpha) {
		alpha = val;
	}
	int numMoves;
	int movelist[150];
	generateCapturesAndPromotions(gs.color, movelist,&numMoves);
	if (numMoves==0 ) return val;

	if (turnSEEOn)
		orderCapturesBySee( movelist, numMoves);
	//else
	   //sorter(movelist, numMoves);

	U64 hash = gs.hash;
	int flags2=gs.flags;

	for (int i=0; i < numMoves; i++)
	{
		//int move = movelist[i];
		int move = getNextMove(i, movelist, numMoves);
		if (move==0) continue;
		make(move);
		numQuiesNodes++;
		if (  isInCheck( 1-gs.color) ) {
			unmake(move, flags2, hash);
			continue;
			// you cannot put yourself in check
		}

		val = -quies(  -beta, -alpha,  depth-1);
		unmake(move, flags2, hash);
		if (val >= beta) {
			return beta;
		}

		// The next three lines test if alpha can be improved by greatest
		// possible matrial swing. This is called delta pruning

		if (!isEndGame() && deltaPruneOn) {
			int BIG_DELTA = QUEEN_VALUE; // queen value
			if (gs.promotion) BIG_DELTA += (QUEEN_VALUE - PAWN_VALUE) ;

			if ( val < alpha - BIG_DELTA ) {
				return alpha;
			}
		}

		if (val > alpha)
			alpha = val;
	}
	return alpha;
}

void orderMovesCaps( int* movelist, int numMoves,  U64 hash, int depth, int hashMove, bool hashFound) {
	for (int i=0; i < numMoves; i++)
	{
		int move = movelist[i];
		if (hashMove==move && hashFound) {
			movelist[i] = 0;   // we already searched it, so set to zero
			break;
		}

	}
}

void orderMoves( int* movelist, int numMoves, int depth, int hashMove, bool hashFound) {
	for (int i=0; i < numMoves; i++)
	{
		int move = movelist[i];

		if ( (hashMove==move) && (hashFound)) {
			movelist[i]=0;
			continue;
		}
		int type =((move >> TYPE_SHIFT) & TYPE_MASK);

		if (type < 6 ) // simple, castling, or pawn push
		{
			bool killerFound=false;

			if (orderByKillers) {
				for (int j=0; j< NUMKILLERS; j++) {
					if (move == killer[depth][j]) {
						movelist[i] = (movelist[i] & ORDERING_CLEAR); // Clear the ordering value
						movelist[i]  = (movelist[i] | ((CAPTURE_SORT_VALS + 1) << ORDERING_SHIFT)); // Change the ordering value and return the new move integer
						killerFound=true;
						break;
					}
				}
			}

			if (killerFound) continue;  //if not, try history

			if (orderByHistory) {
				int from =  (move & SQUARE_MASK);
				int to = ((move >> TO_SHIFT) & SQUARE_MASK);
				// lets order these from 33 to 127
				int historyVal = history[gs.color][from][to];
				if (historyVal > 0) {
					if (historyVal > HISTORY_POINTS) historyVal=HISTORY_POINTS;
					int sortval= min(NO_HISTORY_SORT_VAL,  CAPTURE_SORT_VALS + 2 + (HISTORY_POINTS - historyVal));
					movelist[i] = setOrderingValue ( move, sortval);

				}
			}

		}

	}
	//Util.sort(movelist);
}
void  orderCapturesBySee( int* movelist, int numMoves) {
	int newValue2=INVALID_MOVE;

	for (int i=0; i < numMoves; i++)
	{
		int move = movelist[i];
		int type = ((move >> TYPE_SHIFT) & TYPE_MASK);

		if (type != captureNoPromotion ) {

			continue;
		}
		//capture with promotion already was given a good ordering in movegen
		int myOrderingValue=orderingValue(move);
		// we don't want to waste time by doing a SEE on cheap attackers with expensive victims ( idea from Robert Hyatt/ Crafty )
		// the following are cases of relatively expensive attackers versus victims ie; R X b
		if (myOrderingValue >= 2 && myOrderingValue <= 14) {

			continue;
		}

		int seeValue=  see( move, gs.color); //  captures either get a zero value for no good, or a value from 25 to 975
		if (seeValue==0) {
			movelist[i]=0;
			continue;
		}


		if (seeValue >= 700) {
			newValue2=3;
		}
		else
			if (seeValue >= 600) {
				newValue2=4;
			}
			else
				if (seeValue >= 500) {
					newValue2=5;
				}
				else
					if (seeValue >= 400) {
						newValue2=6;
					}
					else
						if (seeValue >= 300) {
							newValue2=7;
						}
						else
							if (seeValue >= 200) {
								newValue2=8;
							}
							else
								if (seeValue >= 100) {
									newValue2=9;
								}

		movelist[i] = setOrderingValue ( move, newValue2);
	}

	//Util.sort(movelist);
}
bool isCapture(int myMoveType) {
	switch (myMoveType) {
	case captureNoPromotion:
	case capturePromotionQueen:
	case epCapture:
		return true;
	default : return false;
	}

}

void setUsingTime(bool b) {
	usingTime=b;
}

int getNextMove(int i, int moves[], int numMoves) {
	if (numMoves==1)
		return moves[i];
	for (int j = i+1; j < numMoves; j++)
	{
		if ( moves[j] < moves[i] )
		{
			int temp=moves[i];
			moves[i]=moves[j];
			moves[j]=temp;
		}
	}
	return moves[i];
}
