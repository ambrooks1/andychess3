/*
 * movegen.c
 *
 *  Created on: Feb 28, 2014
 *      Author: andrewbrooks
 */

#include "defs.h"
#include "movegen.h"
#include "move.h"
#include "util.h"
#include "magic.h"

 U64 knightMoveArray[64];

  U64 knightMoves (long b)
		{
	  	  	   U64 c = b;
		        b ^= c;
		        b |= (c >> 17) & C64(0x7F7F7F7F7F7F7F7F);
		        b |= (c << 15) & C64(0x7F7F7F7F7F7F7F7F);
		        b |= (c >> 15) & C64(0xFEFEFEFEFEFEFEFE);
		        b |= (c << 17) & C64(0xFEFEFEFEFEFEFEFE);
		        b |= (c >> 10) & C64(0x3F3F3F3F3F3F3F3F);
		        b |= (c <<  6) & C64(0x3F3F3F3F3F3F3F3F);
		        b |= (c >>  6) & C64(0xFCFCFCFCFCFCFCFC);
		        b |= (c << 10) & C64(0xFCFCFCFCFCFCFCFC);
		        return b;
		}

  void initializeMoveGen() {
	  U64 x = 1L;
	  for (int i=0; i < 64; i++) {
		  knightMoveArray[i]=knightMoves(x);
		  //kingMoveArray[i]=kingAttacks(x);

		  //pawnCheckArray[0][i]=getWhitePawnCheckAttack(x);
		  //pawnCheckArray[1][i]=getBlackPawnCheckAttack(x);
		  x= x << 1;
	  }
  }
  //*****************************************************************************
  int  getRookNonCaptures(int cnt, int* moves, U64 rooks, U64  all, U64  target, int fromType)
  {

 	  if ( rooks ) do
 	  {
 		  int victim=-1, moveType, orderValue, move;

 		  U64  attacks;
 		  victim=noVictim;
 		  moveType=simple;
 		  orderValue=DEFAULT_SORT_VAL;
 		  int idx = bitScanForward(rooks); // square index from 0..63
 		  attacks = getRookAttacks(idx, all) & target;

 		  while (attacks)  {
 			  int squareTo = bitScanForward(attacks);

 			  move = 0
 					  | (63-idx)	// from
 					  | ( (63-squareTo) << TO_SHIFT) // to
 					  | (fromType  << PIECE_SHIFT) // piece moving
 					  | (victim << CAPTURE_SHIFT) //piece captured
 					  | ( simple << TYPE_SHIFT) // move type
 					  | (orderValue << ORDERING_SHIFT); // ordering value
 			  moves[cnt++]=move;
 			  attacks = attacks & ( attacks - 1 );
 		  }

 	  }
 	  while (rooks &= rooks-1); // reset LS1B

 	  return cnt;
 }
  int  getBishopNonCaptures(int cnt, int* moves, U64 bishops, U64  all, U64  target, int fromType)
 {

	  if ( bishops ) do
	  {
		  int victim=-1, moveType, orderValue, move;

		  U64  attacks;
		  victim=noVictim;
		  moveType=simple;
		  orderValue=DEFAULT_SORT_VAL;
		  int idx = bitScanForward(bishops); // square index from 0..63
		  attacks = getBishopAttacks(idx, all) & target;

		  while (attacks)  {
			  int squareTo = bitScanForward(attacks);

			  move = 0
					  | (63-idx)	// from
					  | ( (63-squareTo) << TO_SHIFT) // to
					  | (fromType  << PIECE_SHIFT) // piece moving
					  | (victim << CAPTURE_SHIFT) //piece captured
					  | ( simple << TYPE_SHIFT) // move type
					  | (orderValue << ORDERING_SHIFT); // ordering value
			  moves[cnt++]=move;
			  attacks = attacks & ( attacks - 1 );
		  }

	  }
	  while (bishops &= bishops-1); // reset LS1B
	  return cnt;
}

  int getKnightNonCaptures(int cnt, int* moves, U64 knights, U64 target,  int fromType)
  {

	  if ( knights ) do {

		  int victim=-1, orderValue, move;
		  U64 attacks;
		  victim=noVictim;

		  orderValue=DEFAULT_SORT_VAL;
		  int idx = bitScanForward(knights); // square index from 0..63
		  attacks = knightMoveArray[idx] & target;

		  while (attacks)  {
			  int squareTo = bitScanForward(attacks);
			  move = 0
					  | (63-idx)	// from
					  | ( (63-squareTo) << TO_SHIFT) // to
					  | (fromType  << PIECE_SHIFT) // piece moving
					  | (victim << CAPTURE_SHIFT) //piece captured
					  | ( simple << TYPE_SHIFT) // move type
					  | (orderValue << ORDERING_SHIFT); // ordering value
			  moves[cnt++]=move;
			  attacks = attacks & ( attacks - 1 );
		  }
	  }
	  while (knights &= knights-1); // reset LS1B

	  return cnt;
  }

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
