/*
 * movegen.c
 *
 *  Created on: Feb 28, 2014
 *      Author: andrewbrooks
 */

#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "movegen.h"
#include "move.h"
#include "util.h"
#include "magic.h"
#include "assert.h"

U64 knightMoveArray[64];
U64  kingMoveArray[64];
U64 pawnCheckArray[2][64];    //white, black

const int orderingValues[]= {26,26,20,20,14,14,8,8,2,2};
int sortVal[6][5];

int* getCheckEvasionSquares( int color, gameState gs) ;
int* pawnCheck( int color, int kingIdx, gameState gs) ;
int* knightCheck( int color, int kingIdx, gameState gs);
int* isInCheck2( int color, int piece, int type, int kingIdx, U64 allPieces, gameState gs);

U64 kingAttacks (U64 b)
{
	U64 c = b;
	b |= (b >> 1) & 0x7F7F7F7F7F7F7F7FL;
	b |= (b << 1) & 0xFEFEFEFEFEFEFEFEL;
	b |=  b << 8;
	b |=  b >> 8;
	return b ^ c;
}

U64 knightMoves (U64 b)
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

U64 getWhitePawnCheckAttack(U64 king) {
	U64 c = ((king & ~fileA) >> 7 );
	U64 d = ((king & ~fileH) >> 9 );
	return  c | d ;
}

U64 getBlackPawnCheckAttack(U64 king) {
	U64 c = ((king & ~ fileH ) << 7);   //capture
	U64 d = ((king  & ~fileA)  << 9 );  //capture
	return  c | d ;
}

void initializeMoveGen() {
	U64 x = 1L;
	for (int i=0; i < 64; i++) {
		knightMoveArray[i]=knightMoves(x);
		kingMoveArray[i]=kingAttacks(x);

		pawnCheckArray[0][i]=getWhitePawnCheckAttack(x);
		pawnCheckArray[1][i]=getBlackPawnCheckAttack(x);
		x= x << 1;
	}

	//attacker, victim  0,1,2,3,4  = pawn, knight, bishop, rook, queen

	/*
	 		PxQ NxQ BxQ RxQ QxQ KxQ
	 		PxR NxR BxR RxR QxR KxR
	 		PxB NxB BxB RxB QxB KxB
	 		PxN NxN BxN RxN QxN KxN
	 		PxP NxP BxP RxP QxP KxP
	 */

	sortVal[PAWN][PAWN]= 26;
	sortVal[PAWN][KNIGHT]= 11;
	sortVal[PAWN][BISHOP]= 12;
	sortVal[PAWN][ROOK]= 8;
	sortVal[PAWN][QUEEN]= 2;

	sortVal[KNIGHT][PAWN]= 27;
	sortVal[KNIGHT][KNIGHT]= 21;
	sortVal[KNIGHT][BISHOP]= 15;
	sortVal[KNIGHT][ROOK]= 9;
	sortVal[KNIGHT][QUEEN]= 3;

	sortVal[BISHOP][PAWN]= 28;
	sortVal[BISHOP][KNIGHT]= 22;
	sortVal[BISHOP][BISHOP]= 16;
	sortVal[BISHOP][ROOK]= 10;
	sortVal[BISHOP][QUEEN]= 4;

	sortVal[ROOK][PAWN]= 29;
	sortVal[ROOK][KNIGHT]= 23;
	sortVal[ROOK][BISHOP]= 17;
	sortVal[ROOK][ROOK]= 12;
	sortVal[ROOK][QUEEN]= 5;

	sortVal[QUEEN][PAWN]= 30;
	sortVal[QUEEN][KNIGHT]= 24;
	sortVal[QUEEN][BISHOP]= 18;
	sortVal[QUEEN][ROOK]= 13;
	sortVal[QUEEN][QUEEN]= 10;

	sortVal[KING][PAWN]= 31;
	sortVal[KING][KNIGHT]= 25;
	sortVal[KING][BISHOP]= 19;
	sortVal[KING][ROOK]= 13;
	sortVal[KING][QUEEN]= 7;

}
//*****************************************************************************
int  getQueenNonCaptures(int cnt, int* moves, U64 queens, U64  all, U64  target, int fromType)
{

	cnt = getBishopNonCaptures(cnt,moves,queens,all, ~all, fromType);
	cnt = getRookNonCaptures(cnt,moves,queens,all, ~all, fromType);

	return cnt;
}

int  getQueenCaptures(int cnt, int* moves, U64 queens, U64  all, U64  target, int fromType, int *board)
{
	cnt =getBishopCaptures(cnt,moves,queens,all,target, fromType, board);
	cnt = getRookCaptures(cnt,moves, queens,all,target, fromType, board);

	return cnt;
}

int  getRookCaptures(int cnt, int* moves, U64 rooks, U64  all, U64  target, int fromType, int* board)
{

	if ( rooks ) do
	{
		int victim=-1, moveType, orderValue, move;
		int sortType=ROOK;
		if (fromType > 7) sortType=QUEEN;
		U64  attacks;
		victim=noVictim;
		moveType=simple;
		orderValue=DEFAULT_SORT_VAL;
		int idx = bitScanForward(rooks); // square index from 0..63
		attacks = getRookAttacks(idx, all) & target;

		while (attacks)  {
			int squareTo = bitScanForward(attacks);
			victim= board[63-squareTo];
			if (victim > 9 || victim < 0) {
				attacks = attacks & ( attacks - 1 );
				continue;
			}
			orderValue = sortVal[sortType][victim/2];
			move = 0
					| (63-idx)	// from
					| ( (63-squareTo) << TO_SHIFT) // to
					| (fromType  << PIECE_SHIFT) // piece moving
					| (victim << CAPTURE_SHIFT) //piece captured
					| ( captureNoPromotion << TYPE_SHIFT) // move type
					| (orderValue << ORDERING_SHIFT); // ordering value
			moves[cnt++]=move;
			attacks = attacks & ( attacks - 1 );
		}

	}
	while (rooks &= rooks-1); // reset LS1B
	return cnt;
}
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
int  getBishopCaptures(int cnt, int* moves, U64 bishops, U64  all, U64  target, int fromType, int* board)
{

	if ( bishops ) do
	{
		int victim=-1, moveType, orderValue, move;

		U64  attacks;
		victim=noVictim;
		moveType=simple;
		orderValue=DEFAULT_SORT_VAL;
		int sortType=BISHOP;
		if (fromType > 7) sortType=QUEEN;

		int idx = bitScanForward(bishops); // square index from 0..63
		attacks = getBishopAttacks(idx, all) & target;

		while (attacks)  {
			int squareTo = bitScanForward(attacks);
			victim= board[63-squareTo];
			if (victim > 9 || victim < 0) {
				attacks = attacks & ( attacks - 1 );
				continue;
			}
			orderValue = sortVal[sortType][victim/2];
			move = 0
					| (63-idx)	// from
					| ( (63-squareTo) << TO_SHIFT) // to
					| (fromType  << PIECE_SHIFT) // piece moving
					| (victim << CAPTURE_SHIFT) //piece captured
					| ( captureNoPromotion << TYPE_SHIFT) // move type
					| (orderValue << ORDERING_SHIFT); // ordering value
			moves[cnt++]=move;
			attacks = attacks & ( attacks - 1 );
		}

	}
	while (bishops &= bishops-1); // reset LS1B
	return cnt;
}
int getKingNonCaptures(int cnt, int* moves, U64 king, U64 target,  int fromType)
{
	assert(king != 0);
	int victim=-1, orderValue, move;
	U64 attacks;
	victim=noVictim;

	orderValue=DEFAULT_SORT_VAL;
	int idx = bitScanForward(king); // square index from 0..63
	attacks = kingMoveArray[idx] & target;

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

	return cnt;
}
int getKingCaptures(int cnt, int* moves, U64 king, U64 target,  int fromType, int* board)
{
	assert(king != 0);
	int victim=-1, orderValue, move;
	U64 attacks;
	victim=noVictim;

	orderValue=DEFAULT_SORT_VAL;
	int idx = bitScanForward(king); // square index from 0..63
	attacks = kingMoveArray[idx] & target;

	while (attacks)  {
		int squareTo = bitScanForward(attacks);
		victim= board[63-squareTo];
		if (victim > 9 || victim < 0) {
			attacks = attacks & ( attacks - 1 );
			continue;
		}
		orderValue = sortVal[KING][victim/2];
		move = 0
				| (63-idx)	// from
				| ( (63-squareTo) << TO_SHIFT) // to
				| (fromType  << PIECE_SHIFT) // piece moving
				| (victim << CAPTURE_SHIFT) //piece captured
				| ( captureNoPromotion << TYPE_SHIFT) // move type
				| (orderValue << ORDERING_SHIFT); // ordering value
		moves[cnt++]=move;
		attacks = attacks & ( attacks - 1 );
	}

	return cnt;
}

int  getMovesForTheKing(int cnt, int* moves, U64 king, U64 myPieces,  int fromType, int* board, gameState gs) {

	U64 all= gs.bitboard[ALLPIECES];
	U64 enemyPieces=gs.bitboard[WHITEPIECES+(1-gs.color)];

	cnt = getKingCaptures( cnt,moves, gs.bitboard[WK+gs.color],enemyPieces,WK+gs.color, gs.board);
	cnt = getKingNonCaptures(cnt,moves,gs.bitboard[WK+gs.color], ~all,WK+gs.color);
	return cnt;
}

int getKnightCaptures(int cnt, int* moves, U64 knights, U64 target,  int fromType, int *board)
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
			victim= board[63-squareTo];
			if (victim > 9 || victim < 0) {
				attacks = attacks & ( attacks - 1 );
				continue;
			}
			int j = fromType/2;
			int k = victim/2;
			orderValue = sortVal[j][k];
			move = 0
					| (63-idx)	// from
					| ( (63-squareTo) << TO_SHIFT) // to
					| (fromType  << PIECE_SHIFT) // piece moving
					| (victim << CAPTURE_SHIFT) //piece captured
					| ( captureNoPromotion << TYPE_SHIFT) // move type
					| (orderValue << ORDERING_SHIFT); // ordering value
			moves[cnt++]=move;
			attacks = attacks & ( attacks - 1 );
		}
	}
	while (knights &= knights-1); // reset LS1B

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
U64  circular_left_shift(U64 target, int shift){
	return target << shift |  target >> (64-shift);
}


int addPawnCaptureMoves(int cnt, int* moves, int diff, U64 targets, int pieceMoving, int* board, int moveType){
	int orderValue, to, from, victim2, move;

	while (targets != 0){
		to =  bitScanForward(targets);
		from = (to - diff) % 64;

		victim2 = board[63 - to];
		if (victim2 < 0 || victim2 > 9) {
			targets &= targets - 1;
			continue;
		}
		orderValue=orderingValues[victim2];

		move = 0
				|  63 -from	// from
				| ( (63 -to) << TO_SHIFT) // to
				| (pieceMoving  << PIECE_SHIFT) // piece moving
				| (victim2 << CAPTURE_SHIFT) //piece captured
				| (moveType << TYPE_SHIFT) // move type
				| (orderValue << ORDERING_SHIFT); // ordering value
		moves[cnt++] =move;
		targets &= targets - 1;
	}
	return cnt;
}

int addPawnCaptureMovesEP(int cnt, int* moves, int diff, U64 targets, int pieceMoving, int side){
	int victim2[] ={  BP, WP  };

	int lowerbound[] = { 32, 24 };
	int upperbound[] = { 39, 31 };
	int from, to, move;

	while (targets != 0){
		to =  bitScanForward(targets);
		from = (to - diff) % 64;

		if (( from < lowerbound[side]) || ( from > upperbound[side])) {    // we need fifth rank
			targets &= targets - 1;
			continue;
		}

		move = 0
				|  (63 -from)	// from
				| ( (63 -to) << TO_SHIFT) // to
				| (pieceMoving  << PIECE_SHIFT) // piece moving
				| (victim2[side] << CAPTURE_SHIFT) //piece captured
				| (epCapture << TYPE_SHIFT) // move type
				| (26 << ORDERING_SHIFT); // ordering value
		moves[cnt++] =move;
		targets &= targets - 1;
	}
	return cnt;
}

int addPawnAttacks(int cnt, int* moves, U64 pawns,  U64 enemy, int side, int pieceMoving, int flags, int* board) {
	//just for white for the moment

	U64 attacks, ep_attacks, promotions, targets;
	//int epSquare = EnPassant.getEPSquare(flags);
	int epSquare = flags >>EP_SHIFT;
	const int attackDiffs[2][2]       = {{7, -9}, {9, -7}};
	const U64 promotions_mask[] = {RANK8, RANK1};
	const U64 pawnAttackFileMask[]    = {~fileH, ~fileA};

	// CALCULATE ATTACKS FOR LEFT, RIGHT
	for (int dir = 0; dir < 2; dir++){
		int diff =  attackDiffs[dir][side];

		U64 x = pawns & pawnAttackFileMask[dir];
		targets = x << diff | x >> (64-diff);

		attacks = enemy & targets;
		if (attacks != 0)
			cnt = addPawnCaptureMoves(cnt, moves, diff, attacks & (~promotions_mask[side]),   pieceMoving,  board, captureNoPromotion);


		if ( epSquare != 0) {
			U64 epMask = 1L << epSquare;
			ep_attacks = targets & epMask;
			cnt = addPawnCaptureMovesEP(cnt, moves, diff, ep_attacks,  pieceMoving, side);
		}

		promotions = attacks & promotions_mask[side];
		if (promotions != 0)
			cnt= addPawnCaptureMoves(cnt, moves, diff, promotions, pieceMoving,  board, capturePromotionQueen);

	}
	return cnt;
}
int addPawnPushMoves(int cnt, int* moves, int diff, U64 targets,  int moveType, int pieceMoving, int sortOrder){
	int from,to,move;

	while (targets != 0){
		to =  bitScanForward(targets);

		from = (to - diff) % 64;

		move = 0
				|  63 -from	// from
				| ( (63 -to) << TO_SHIFT) // to
				| (pieceMoving  << PIECE_SHIFT) // piece moving
				| (0 << CAPTURE_SHIFT) //piece captured
				| (moveType << TYPE_SHIFT) // move type
				| (sortOrder << ORDERING_SHIFT); // ordering value

		moves[cnt++] =move;
		targets &= targets - 1;
	}
	return cnt;
}
int getPawnCapturesAndPromotions(int cnt, int* moves, U64 pawns, U64 all,
		U64 enemy, int side, int pieceMoving, int flags, int* board)  {

	const int pushDiffs[]                   = {8, -8};

	const U64 promotions_mask[] = {RANK8, RANK1};
	int diffs2=pushDiffs[side];

	U64 free_squares = ~all;
	U64 pushes=0;
	pushes = circular_left_shift(pawns, diffs2) & free_squares;
	U64 promotions = pushes & promotions_mask[side];
	if (promotions != 0)
		cnt=addPawnPushMoves(cnt, moves, diffs2, promotions, simplePromotionQueen,pieceMoving, 1);
	cnt=addPawnAttacks(cnt, moves, pawns, enemy, side, pieceMoving, flags,  board);
	return cnt;
}

void generateCheckEvasionMoves(int color, gameState gs, int moves[], int *cntMoves) {
	const int pushDiffs[]                   = {8, -8};
	// note that we must still verify the legality of the king and pawn moves, after make,
	//but for the the other generated moves, this is not needed
	U64 all=gs.bitboard[ALLPIECES];
	int cnt=0;

	cnt =getMovesForTheKing( cnt,moves,gs.bitboard[WK+color],gs.bitboard[WHITEPIECES+color],WK+color, gs.board, gs);

	// the first item in squares is the attacking piece
	// we try to generate moves which capture this piece, followed by interposing moves

	int* squares = getCheckEvasionSquares( color, gs);
	if (squares==0) {
		*cntMoves=cnt;
		return;
	}

	U64 pawns = gs.bitboard[WP+color];
	U64 bishops=gs.bitboard[WB+color];
	U64 rooks=gs.bitboard[WR+color];
	U64 knights=gs.bitboard[WN+color];
	U64 queens=gs.bitboard[WQ+color];

	U64 attacker = C64(1) << squares[0];

	if (squares != 0) {
		cnt=addPawnAttacks(cnt, moves, pawns, attacker, color, WP+color, gs.flags,  gs.board);
		cnt =getKnightCaptures(cnt,moves, knights, attacker,WN+color, gs.board);
		cnt =getBishopCaptures(cnt,moves,bishops,all,attacker,WB+color, gs.board);
		cnt = getRookCaptures(cnt,moves,rooks,all,attacker,WR+color, gs.board);
		cnt = getQueenCaptures(cnt,moves,queens,all,attacker,WQ+color, gs.board);
	}
	// the other possible squares, if they exist, should be target squares for noncaptures to block the check
	int diffs2=pushDiffs[color];

	int i=0;
	while (squares[i])
	{
		U64 interveningSquare=1L <<squares[i];
		U64 interveningSquare2=1L <<squares[i];
		interveningSquare2 = interveningSquare2 | ( 1L << ( squares[i] -diffs2));

		cnt = getPawnPushes(cnt, moves, pawns,~interveningSquare2, color, WP+color) ;
		cnt = getKnightNonCaptures(cnt, moves, knights, interveningSquare, WN+color) ;
		cnt = getBishopNonCaptures(cnt, moves, bishops, all, interveningSquare, WB+color) ;
		cnt = getRookNonCaptures(cnt,   moves, rooks, all, interveningSquare,WR+color) ;
		cnt = getQueenNonCaptures(cnt, moves, queens, all, interveningSquare,WQ+color) ;
		i++;
	}

	*cntMoves=cnt;

}

int* getCheckEvasionSquares( int color, gameState gs) {

	U64 allPieces = gs.bitboard[ALLPIECES];
	U64 king = gs.bitboard[WK+color];
	int kingIdx = bitScanForward(king);

	int* result1 = isInCheck2( color,WR, ROOK_ATTACK, kingIdx, allPieces, gs);
	if ( result1 != 0) {
		return result1;
	}
	int* result2 = isInCheck2( color,WB, BISHOP_ATTACK, kingIdx, allPieces, gs);
	if ( result2 != 0)  {
		return result2;
	}

	int* result3 = isInCheck2( color,WQ, ROOK_ATTACK, kingIdx, allPieces, gs);
	if ( result3 != 0) {
		return result3;
	}
	int* result4 = isInCheck2( color,WQ, BISHOP_ATTACK, kingIdx, allPieces, gs);
	if ( result4 != 0) {
		return result4;
	}

	int* result5=  knightCheck( color, kingIdx,gs);
	if ( result5 != 0) {
		return result5;
	}

	int* result6 = pawnCheck( color, kingIdx,gs);
	if ( result6 != 0) {
		return result6;
	}

	return 0;
}


int* pawnCheck( int color, int kingIdx, gameState gs) {
	U64 pawns = gs.bitboard[WP + (1-color)];
	U64 x = pawnCheckArray[(1-color)][kingIdx];

	if  (( x  & pawns) != 0 ) {
		while (pawns)
		{
			int idx = bitScanForward(pawns);
			U64 justOnePawn = C64(1) << idx;

			if ( (x & justOnePawn ) != 0 ) {
				int * out = (int *) calloc(1, sizeof(int));
				out[0]=idx;
				return out;
			}
			pawns = pawns & ( pawns - 1 );
		}
	}
	return 0;
}

int* knightCheck( int color, int kingIdx, gameState gs) {

	U64 knights = gs.bitboard[WN + (1-gs.color)];
	U64 x = knightMoveArray[kingIdx];

	if ( (x & knights ) != 0 )  {

		while (knights)
		{
			int idx = bitScanForward(knights);

			U64 justOneKnight = C64(1) << idx;

			if ( (x & justOneKnight ) != 0 ) {
				int * out = (int *) calloc(1, sizeof(int));
				out[0]=idx;
				return out;
			}
			knights = knights & ( knights - 1 );

		}
	}
	return 0;
}

int* isInCheck2( int color, int piece, int type, int kingIdx, U64 allPieces, gameState gs) {
	// returns array of squares
	// if color is in check return keysquare array
			// first square is the attacking piece

	U64 enemyPiece  = gs.bitboard[piece+ ( 1-color)] ;    // could be two rooks, for example
	//long enemyBishops = gs.bitboard[GameState.WQ+ ( 1-color)] ;
	U64 attacks1;

	if (type == ROOK_ATTACK) {
		attacks1 = getRookAttacks(kingIdx,allPieces) &  enemyPiece;
	}
	else {
		attacks1 = getBishopAttacks(kingIdx,allPieces) &  enemyPiece;
	}
	int out[8];

	int k=0;
	if (attacks1!= 0)
	{
		while (enemyPiece)
		{
			int idx = bitScanForward(enemyPiece);
			U64 oneEnemyPiece = 1L << idx;

			//System.out.println("attack idx " + idx);
			if ((oneEnemyPiece & attacks1) != 0 ) {

				out[k++]=idx;

				U64 inBetweenSquares = inBetweenWithoutLookup(idx, kingIdx);

				while (inBetweenSquares){
					int idx2 = bitScanForward(inBetweenSquares);
					//System.out.println("rook in between idx " + idx);
					out[k++]=idx2;

					inBetweenSquares = inBetweenSquares & ( inBetweenSquares - 1 );
				}
				break;
			}
			enemyPiece = enemyPiece & ( enemyPiece - 1 );
		}
	}

	if (k > 0) {
		int * out2 = (int *) calloc(k, sizeof(int));

		memcpy(out2, out, sizeof(out) );
		return out2;
	}
	return 0;
}

