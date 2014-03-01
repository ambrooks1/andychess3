/*
 * movegen.c
 *
 *  Created on: Feb 28, 2014
 *      Author: andrewbrooks
 */

#include "defs.h"
#include "movegen_defs.h"
#include "move.h"
#include "util.h"

int getPawnPushes(int cnt, int* moves, U64 pawns, U64 all, int side, int pieceMoving)
{
	const int pushDiffs[2]                   = {8, -8};

	const U64 promotions_mask[2] = {RANK8, RANK1};
	const U64 start_row_plus_one_mask[2] = {RANK3,RANK6};
	int diffs2=pushDiffs[side];

	U64 free_squares = ~all;

	// ADD SINGLE PUSHES
	U64 pushes = ( pawns << diffs2 | pawns >> (64-diffs2)) & free_squares;

	U64 targets3=  pushes & (~promotions_mask[side]);

	int from, to,move, sortOrder=DEFAULT_SORT_VAL;

	while (targets3 != 0){
		to =  bitScanForward(targets3);

		from = (to - diffs2) % 64;

		move = 0
				|  (63 -from)	// from
				|  ( (63 -to) << TO_SHIFT) // to
				| (pieceMoving  << PIECE_SHIFT) // piece moving
				| (0 << CAPTURE_SHIFT) //piece captured
				| ( simple << TYPE_SHIFT) // move type
				| (sortOrder << ORDERING_SHIFT); // ordering value

		moves[cnt++] =move;
		targets3 &= targets3 - 1;
	}
	U64 target2= pushes & start_row_plus_one_mask[side];
	U64 double_pushes = ( target2 << diffs2 | target2 >> (64-diffs2)) & free_squares;

	sortOrder=50;
	int diffs3=2*diffs2;

	while (double_pushes != 0){
		to =  bitScanForward(double_pushes);

		from = (to - diffs3) % 64;

		move = 0
				|  (63 -from)	// from
				| ((63 -to) << TO_SHIFT) // to
				| (pieceMoving  << PIECE_SHIFT) // piece moving
				| (0 << CAPTURE_SHIFT) //piece captured
				| (doublePawnMove << TYPE_SHIFT) // move type
				| (sortOrder << ORDERING_SHIFT); // ordering value

		moves[cnt++] =move;
		double_pushes &= double_pushes - 1;
	}

	return cnt;
}
