/*
 * SEE.c
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */


/****
 *
 * A Static Exchange Evaluation (SEE) examines the consequence of a series of exchanges
 * on a single square after a given move, and calculates the likely evaluation change
 * (material) to be lost or gained, Donald Michie coined the term swap-off value.
 *
 * A positive static exchange indicates a "winning" move.
 *  For example, PxQ will always be a win, since the Pawn side can choose to stop the exchange after its Pawn is recaptured,
 *   and still be ahead.
 *    Some programs optimize the SEE function to only return a losing or equal/winning flag, since they only use SEE to determine
 *     if a move is worth searching and do not need the actual value.
 *      SEE is useful in move ordering, futility pruning and especially in quiescence search in conjunction with delta pruning,
 *       as well to reduce "bad" captures and checks [1] .
 */
#include <stdlib.h>
#include <stdio.h>
#include "defs.h"
#include "movegen.h"
#include "magic.h"
#include "util.h"
#include "gamestate.h"
extern gameState gs;

void getRegularAttackers( int to, U64 allPieces,
		U64 queen[], U64 rook[], U64 bishop[], U64 knight[], int ptr[], int attackers[][16]);

extern U64 knightMoveArray[64];
extern U64  kingMoveArray[64];

int max(int x, int y) {
	if (x > y) return x;
	return y;
}
U64  xrayRookAttacks(U64 occ, U64 blockers, int rookSq) {
	U64 attacks = getRookAttacks(rookSq, occ);
	blockers &= attacks;
	return attacks ^ getRookAttacks( rookSq, occ ^ blockers);
}

U64  xrayBishopAttacks(U64 occ, U64 blockers, int bishopSq) {
	U64 attacks = getBishopAttacks(bishopSq, occ );
	blockers &= attacks;
	return attacks ^ getBishopAttacks(bishopSq, occ ^ blockers );
}

void insertXRayAttacker(int side, int attackers[][16], int ptr[],
		int xrayAttacker, int blocker) {

	for (int j=0; j <= ptr[side]; j++) {      // find the blocker on the attacker stack
		if (attackers[side][j] == blocker) {
			ptr[side]++;
			for (int k = ptr[side];  k > j; k--) {
				attackers[side][k]=attackers[side][k-1];
			}
			attackers[side][j] = xrayAttacker;
			break;
		}
	}
}
void getXRayAttackerRook( U64 all, int i, U64 blockers[], int attackingColor, int vertXRayAttacker[][2]) {
	//blockers should be rooks, queens
	for (int j=0; j < 2; j++) {
		U64 xray = xrayRookAttacks(all ,blockers[j], i) ;
		int victimSquare=63-i;
		int victim= gs.board[victimSquare];
		if (victim == -1 || victim == attackingColor) return ;


		while (xray) {
			int idx = bitScanForward(xray);
			int attackerSquare=63-idx;

			int attacker = gs.board[attackerSquare];

			if ( (attacker != (WR+attackingColor)) && ( attacker != (WQ+attackingColor)   )     )  {
				xray = xray & ( xray - 1 );
				continue;
			}

			int blocker=WR+attackingColor;
			if (j==1) blocker=WQ+attackingColor;

			vertXRayAttacker[attackingColor][0]=attacker;
			vertXRayAttacker[attackingColor][1]=blocker;
			return;
		}
	}

}

void  getXRayAttackerBishop( U64 all, int i, U64 blockers[], int attackingColor, int diagXRayAttacker[][2]) {
	//blockers should be queens, rooks

	for (int j=0; j < 2; j++) {
		U64 xray = xrayBishopAttacks(all ,blockers[j], i) ;
		int victimSquare=63-i;
		int victim= gs.board[victimSquare];
		if (victim == -1 || victim == attackingColor) return ;


		while (xray) {
			int idx = bitScanForward(xray);
			int attackerSquare=63-idx;

			int attacker = gs.board[attackerSquare];

			if ( (attacker != (WB+attackingColor)) && ( attacker != (WQ+attackingColor)   )     )  {
				xray = xray & ( xray - 1 );
				continue;
			}

			int blocker;

			if (j==0) blocker=WB+attackingColor;
			else
				blocker=WQ+attackingColor;
			diagXRayAttacker[attackingColor][0]=attacker;
			diagXRayAttacker[attackingColor][1]=blocker;
			return;

		}
	}

}


int other(int side) {
	if (side==WHITE) return BLACK; else return WHITE;
}


// This is the main routine used by search
int see( int move, int side)  // the side that is moving
{
	// does not work for stack of three XRay attackers/defenders, only for stack of two
	// does not work for xray bishop/queen behind attacking pawn  (:(

	//int from2 = Move2.fromIndex(move);
	//int to2 = Move2.toIndex(move);

	int to2 = ((move >> 6) & 63);
	int victimSquare = 63 - to2;
	const int valueMap[] = { 100, 100, 325,  325,
			350, 	350,  500,
			500, 975 ,975};


	U64 all =  gs.bitboard[ALLPIECES];

	U64 queens[2];
	U64 bishops[2];
	U64 rooks[2];
	U64 knights[2];

	for (int i=0; i < 2; i++) {
		queens[i] = gs.bitboard[WQ+i];
		rooks[i] = gs.bitboard[WR+i];
		bishops[i] = gs.bitboard[WB+i];
		knights[i] = gs.bitboard[WN+i];
	}

	//U64 pawns = gs.bitboard[WR+side];

	U64 rookBlockers[2][2] = { { rooks[0], queens[0]}, { rooks[1], queens[1]}};

	U64 bishopBlockers[2][2] = { { bishops[0], queens[0]}, { bishops[1], queens[1]}};

	 // colors, pieces
	int attackers[2][16] =
	{
			{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
			{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
	};

	//get regular attackers
	int ptr[2] = { -1,-1};
	getRegularAttackers( victimSquare, all,
			queens, rooks, bishops, knights, ptr, attackers);

	// move the original attackers to the top of the stack
	// 'side' is the attacking side

	int from2 =  (move & 63);
	int attacker = gs.board[from2];    //the initial attacker
	for (int i=0; i <= ptr[side]; i++) {
		if (attackers[side][i] == attacker) {

			for (int j = i; j < ptr[side]; j++) {
				attackers[side][j]=attackers[side][j+1];
			}
			attackers[side][ptr[side]] = attacker;
			break;
		}
	}
	//put the xRayAttackers into the stack, but behind their associated regular attackers
	//put the xRayAttackers into the stack, but behind their associated regular attackers

	 //white, black and attacker, blocker
	int diagXRayAttacker[2][2] = {  { -1, -1 }, {-1, -1} };

	int vertHorizXRayAttacker[2][2] = {  { -1, -1 }, {-1, -1} };

	for (int i=0; i < 2; i++) {
		getXRayAttackerBishop( all, victimSquare, bishopBlockers[i], i, diagXRayAttacker);


		if (diagXRayAttacker[i] != 0) {

			int attacker2= diagXRayAttacker[i][0];
			int blocker2= diagXRayAttacker[i][1];
			insertXRayAttacker(side, attackers,ptr, attacker2, blocker2);
		}
		getXRayAttackerRook( all, victimSquare, rookBlockers[i], i, vertHorizXRayAttacker);
		if (vertHorizXRayAttacker[i] != 0) {
			int attacker2= vertHorizXRayAttacker[i][0];
			int blocker2= vertHorizXRayAttacker[i][1];
			insertXRayAttacker(side, attackers,ptr, attacker2, blocker2);
		}
	}
	//finally, evaluate

	int victim = gs.board[to2];      // the initial victim
	if (victim==-1)   return 0;

	if (ptr[side]==-1) return 0;

	int value=0;
	int origSide=side;

	while ( ptr[side] >= 0 ) {
		if (victim == -1 || victim >= 10)  break;
		int piece= attackers[side][ptr[side]];
		ptr[side]--;
		int value2 = valueMap[victim];

		if (side == 0) {
			value += value2;
		}
		else {
			value -= value2;
		}
		victim=piece;
		side=other(side);
	}

	int retval=value;
	if (origSide == 1) retval=-value;

	if (value == 0 ) return 1;

	return max(   0, retval );
	// System.out.println("value= " + value);

	//Make sure not to use for enpassant capture !!

}

void getRegularAttackers( int to, U64 allPieces,
		U64 queen[], U64 rook[], U64 bishop[], U64 knight[], int ptr[], int attackers[][16]){

	//int[2][16] attackers;     colors, pieces

	for (int i=0; i < 2; i++ ) {
		for (int j=0; j < 16; j++ ) {
			attackers[i][j]=-1;
		}
	}
	U64 kingAttacks =  kingMoveArray[to];
	U64 bishopAttacks= getBishopAttacks(to, allPieces);
	U64 rookAttacks = getRookAttacks(to,allPieces);
	U64 knightAttacks = knightMoveArray[to];

	for (int side=0; side < 2; side++) {
		if ( (kingAttacks & gs.bitboard[WK+side]) != 0 )
			attackers[side][ ++ptr[side]]= WK+side;

		if ((bishopAttacks & queen[side])!= 0)
			attackers[side] [++ptr[side]]= WQ+side;
		else {
			if ((rookAttacks & queen[side] )!= 0)
				attackers[side] [++ptr[side]]= WQ+side;
		}

		if ((rookAttacks & rook[side] )!= 0)
			attackers[side][ ++ptr[side]]= WR+side;

		if ((bishopAttacks & bishop[side] )!= 0)
			attackers[side][ ++ptr[side]]= WB+side;

		if ( (knightAttacks & knight[side]) != 0 )
			attackers[side][ ++ptr[side]]= WN+side;
	}

	U64 piece = 1L << to;

	U64 c = ((piece & ~fileA) >> 7 );
	if ( (c  & gs.bitboard[WP]) != 0 )
		attackers[0][++ptr[0]]= WP;

	U64 d = ((piece & ~fileH) >> 9 );
	if ( (d  & gs.bitboard[WP]) != 0 )
		attackers[0][++ptr[0]]= WP;

	c = ((piece & ~ fileH ) << 7);   //capture
	if (( c & gs.bitboard[BP]) != 0 )
		attackers[1][++ptr[1]]= BP;

	d = ((piece  & ~fileA)  << 9 );  //capture
	if (( d & gs.bitboard[BP]) != 0 )
		attackers[1][++ptr[1]]= BP;

}

int evaluate2(int side, int victim, int attackers[][16],  int ptr[]) {
	const int valueMap[] = { 100, 100, 325,  325,
			350, 	350,  500,
			500, 975 ,975, 9999, 9999};

	if (victim==-1)   return 0;

	if (ptr[side]==-1) return 0;

	int value=0;
	int origSide=side;

	while ( ptr[side] >= 0 ) {
		if (victim == -1)  break;
		int piece= attackers[side][ptr[side]];
		ptr[side]--;
		int value2 = valueMap[victim];

		if (side == 0) {
			value += value2;
		}
		else {
			value -= value2;
		}
		victim=piece;
		side=other(side);
	}

	int retval=value;
	if (origSide == 1) retval=-value;

	if (value == 0 ) return 1;

	return max(   0, retval );
}




