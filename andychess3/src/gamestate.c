/*
 ============================================================================
 Name        : andychess3.c
 Author      : Andrew Brooks
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "assert.h"
#include "defs.h"
#include "util.h"
#include "movegen.h"
#include "move.h"
#include "magic.h"
#include "eval.h"
#include "gamestate.h"

const U64 XYA =  C64(6);
const U64 XYB =  C64(112);
const U64 XYC =  C64(432345564227567616);
const U64 XYD =  C64(8070450532247928832);

const U64 castlingBitboards[] =  {XYA, XYB, XYC, XYD};
const U64 minedBitboards[2][2] = { { C64(4), C64(16)}, {  C64(288230376151711744),  C64(1152921504606846976)}};
const int castleRook[]={WR,BR};
const int valueMap[] = { PAWN_VALUE, PAWN_VALUE, KNIGHT_VALUE,  KNIGHT_VALUE,
		BISHOP_VALUE, 	BISHOP_VALUE,   ROOK_VALUE, ROOK_VALUE,  QUEEN_VALUE , QUEEN_VALUE};

const int pp[]={WQ, BQ};
int kingSideFlags[2];

int queenSideFlags[2];

const int rookHomeQS[] = { 56, 0 };
const int rookHomeKS[] = { 63, 7};
const int kingHome[] = { 60, 4 };
const int king[] = { WK,BK };
const int ksFromRook[]={63, 7};
const int ksToRook[]  ={61, 5};

const U64 ksRookFrom[]={ C64(1) , C64(72057594037927936) };
const U64 ksRookTo[]={ C64(4) , C64(288230376151711744) };
const int  qsFromRook[]={56, 0};
const int  qsToRook[]  ={59, 3};
const U64 qsRookFrom[]={ C64(128) , C64(-9223372036854775808)  };
const U64 qsRookTo[]={ C64(16) , C64(1152921504606846976) };
const int pawn[]={WP, BP};
const int lowerBound[] = {8, 48};
const int upperBound[] = { 15, 55};
const int offset2[]={8, -8};
const U64 fook[] = { C64(1) ^ C64(4),  C64(72057594037927936) ^ C64(288230376151711744)};
const U64 fook2[] = { C64(128) ^ C64(16),  C64(-9223372036854775808) ^ C64(1152921504606846976)};

extern U64 knightMoveArray[64];
extern U64  kingMoveArray[64];
extern U64 pawnCheckArray[2][64];    //white, black

gameState gs = { .initialized=false };

void printMove(int move);

bool isEndGame() {
		  return (gs.material[WHITE]  + gs.material[BLACK] < ENDGAME_MATERIAL_VALUE)? true: false;
}

bool isSameSquareRecapture() {
		    if (gs.moveCounter < 2)  return false;
			int lastVictim     = gs.capturedPieces[gs.moveCounter -1];
			int previousVictim = gs.capturedPieces[gs.moveCounter -2];
			if (lastVictim == -1 || previousVictim == -1 ) return false;
			int value1 = valueMap[lastVictim];
			int value2 = valueMap[previousVictim];
			if ( value1 != value2)  return false;
			int move1 = gs.moveHistory[gs.moveCounter -1];
			int move2 = gs.moveHistory[gs.moveCounter -2];

			int toSquare1 =  ((move1 >> TO_SHIFT) & SQUARE_MASK);
			int toSquare2 =  ((move2 >> TO_SHIFT) & SQUARE_MASK);

		    if (toSquare1==toSquare2) {
		    	return true;
		    }
		    return false;
	}

//**********************************************************************
void initializeFlags() {
	U64 negWks = ~ (  C64(1)  << WKSIDE);
	U64 negWqs = ~ (  C64(1)  << WQSIDE);
	U64  negBks = ~ (  C64(1)  << BKSIDE);
	U64  negBqs = ~ (  C64(1)  << BQSIDE);

	kingSideFlags[0]= negWks;
	kingSideFlags[1]= negBks;

	queenSideFlags[0]= negWqs;

	queenSideFlags[1]= negBqs;
}
U64 rand64() {  //from CPW engine
	return rand() ^ ((U64)rand() << 15) ^ ((U64)rand() << 30) ^ ((U64)rand() << 45) ^ ((U64)rand() << 60);
}
void initializeZobrist() {

	gs.side = rand64();

	for(int i = 0; i < 4; i++)
	{
		gs.W_CASTLING_RIGHTS[i] = rand64();
		gs.B_CASTLING_RIGHTS[i] = rand64();
	}

	for (int i=0; i < 12; i++) {
		for (int k=0; k < 64; k++) {
			gs.pieces[i][k] = rand64();       // WP, 32  or BK, 54  for example
		}
	}

	for (int i=0; i < 64; i++) {
		gs.en_passant[i] = rand64();
	}


}
void printBoard() {
	printf("Here comes the board\n");
	for (int i=0; i < 64; i++) {
		if (i % 8 == 0) {
			printf("\n");
		}
		printf("%c", reverseConvertPiece(gs.board[i]));
	}
	printf("\n");
}
//**********************************************************************
void setAllBitboards() {
	for (int i=0; i < 12; i++) {
		gs.bitboard[i]=0;
	}

	for (int i=0; i < 64; i++) {
		int piece = gs.board[i];
		assert(( piece >= WP && piece <= BK) || piece == EMPTY);
		if (piece != EMPTY) {
			//printf("piece %d i= %d\n", piece, i);
			gs.bitboard[piece] = setBitLong(gs.bitboard[piece], 63-i );

		}
	}
	gs.bitboard[WHITEPIECES]= gs.bitboard[WP] | gs.bitboard[WN] | gs.bitboard[WB] | gs.bitboard[WR] | gs.bitboard[WQ] | gs.bitboard[WK];
	gs.bitboard[BLACKPIECES]= gs.bitboard[BP] | gs.bitboard[BN] | gs.bitboard[BB] | gs.bitboard[BR] | gs.bitboard[BQ] | gs.bitboard[BK];
	gs.bitboard[ALLPIECES] = gs.bitboard[WHITEPIECES] | gs.bitboard[BLACKPIECES];

	/*for (int i=0; i < 14; i++) {
		printf("bitboard %d  value %llu\n", i, gs.bitboard[i]);
	}*/
}
//**********************************************************************
void displayBitboards() {

	for (int i=0; i < 12; i++) {
		U64 x = gs.bitboard[i];
		printf("Processing bitboard %d\n", i);

		if ( x ) do {
			int idx = bitScanForward(x); // square index from 0..63
			printf("piece on square %s\n",   getSquareFromIndex(63-idx) );
		}
		while (x &= x-1); // reset LS1B
	}
}

//**********************************************************************
int setEnPassantValue(int irrevState, int value) {
	irrevState = (irrevState & EP_CLEAR); // Clear the  value
	return (irrevState | (value << EP_SHIFT)); // Change the value and return the new irrev state integer
}
//**********************************************************************
void fenToCastlingPrivilege(char *castlingS) {
	for (int i=0; i < 4; i++)
		gs.flags =  clearBit(gs.flags,i);

	if (strcmp("-",castlingS) == 0) { //true
		//do nothing
	}
	else {
		if ( strstr(castlingS, "K") != NULL){
			gs.flags = setBit(gs.flags,WKSIDE);
		}
		if ( strstr(castlingS, "Q") != NULL){
			gs.flags = setBit(gs.flags,WQSIDE);
		}
		if ( strstr(castlingS, "k") != NULL){
			gs.flags = setBit(gs.flags,BKSIDE);
		}
		if ( strstr(castlingS, "q") != NULL){
			gs.flags = setBit(gs.flags,BQSIDE);
		}

	}
}

//**********************************************************************
void convertRowsToBoard(char* rows[8]) {

	int index=0;
	//printf("convertRowsToBoard\n");
	for (int i=0; i < 8; i++) {

		char *p = rows[i];
		// printf("GO %s\n", p);

		char* c = p;
		while (*c){
			int m = *c;    // the character '1' is int 49 for example, in the ascii table

			int ia = m - '0';
			// printf("m=%c\n", m);
			if (isalpha(m)) {
				gs.board[index++] = convertPiece(*c);
			}
			else{
				if (isdigit(m)) {
					for (int k=0; k< ia; k++ )
						gs.board[index++]= EMPTY;
				}
			}
			c++;
		} //end while
	}  //end for
	//printf("Finished convertRowsToBoard\n");
}
//**********************************************************************


void parseFen( char fen[]) {
    assert(gs.initialized);

	gs.positional[WHITE]=0;
	gs.positional[BLACK]=0;

	char *cp = strdup(fen);
	char *ch, *ch2;

	char* rows[8];

	//printf("Split \"%s\"\n", fen);

	ch = strtok(fen, " ");
	int i=0;
	while (ch != NULL) {
		if (i==0)
		{
			int j=0;
			ch2 = strtok(ch, "\\/");
			while ( ch2 != NULL) {
				//printf("%s\n", ch2);
				rows[j++]=ch2;
				ch2 = strtok(NULL, "\\/");
			}
		}

		ch = strtok(NULL, " ");
		i++;
	}
	// printf("Split \"%s\"\n", cp);

	i=0;

	ch = strtok(cp, " ");
	while (ch != NULL) {

		/*  if (i > 0 ) {
			  printf("%s\n", ch);
		  }*/

		switch(i) {
		case 1:       // color to play w or b
			// printf("\n color %s\n", ch);

			if (strcmp(ch, "w")==0)  //true
			{
				// printf("color is white\n");
				gs.color=WHITE;
			}
			else {
				//printf("color is black\n");
				gs.color=BLACK;
			}
			break;
		case 2:
			//printf("\n castling privileges %s\n", ch);
			fenToCastlingPrivilege(ch);
			break;
		case 3:
			//printf("\n en passant %s\n", ch);
			if (strcmp("-", ch) == 0) {  //true
				gs.flags = setEnPassantValue(gs.flags, 0);
			}
			else {     // like e3 or d6
				int index =  63 - getIndexFromSquare(ch);
				// printf("\n en passant index %d\n", index);
				gs.flags = setEnPassantValue(gs.flags, index);
			}
			break;
		}

		ch = strtok(NULL, " ");
		i++;
	}

	convertRowsToBoard(  rows);
	//printf("Doing setAllBitboards\n");
	setAllBitboards();
	//printf("Finished Doing setAllBitboards\n");


	gs.material[WHITE]=materialValue(WHITE);

	gs.material[BLACK]=materialValue(BLACK);

	for (int i = 0; i < 6; i++)  {
		if (i % 2 == 0) {
			int whitePos = addPositionalValueForType( i);
			//printf("whitepos %d\n", whitePos);
			gs.positional[WHITE] += whitePos;
		}
		else {
			int blackPos = addPositionalValueForType( i);
			//printf("blackPos %d\n", blackPos);
			gs.positional[BLACK] += blackPos;
		}
		//printf("whitepositional %d blackpositional %d\n", gs.positional[WHITE], gs.positional[BLACK]);
	}

	//printf("Doing calc hash\n");
	gs.hash= calcHash();
	//printf("Finished Doing calc hash\n");
	//printf("Finished parse fen\n");
}

//**********************************************************************
int materialValue(int color) {      // only used for initializing a state  , not used after make
	int pawnVal =   PAWN_VALUE * ( popCount(gs.bitboard[WP+color])  );
	int knightVal = KNIGHT_VALUE * ( popCount(gs.bitboard[WN+color])  );
	int bishopVal = BISHOP_VALUE * ( popCount(gs.bitboard[WB+color]));
	int rookVal =   ROOK_VALUE * ( popCount(gs.bitboard[WR+color])  );
	int queenVal =  QUEEN_VALUE * ( popCount(gs.bitboard[WQ+color])  );
	int val = pawnVal + knightVal + bishopVal + rookVal + queenVal;
	return val;
}

int getMaterialValue() {    //just used for testing
	return materialValue(gs.color) - materialValue(1-gs.color);
}
bool canCastle(int index, U64 allPieces) {
	/*To check whether castling is legal, we have to check whether
			the squares between the King and the Rook in question, are empty.
			This is easily done by just ANDing a pre- computed
			bitboard of these squares with the occupied bitboard.

			Also, you cannot castle out of check
	 */

	if  (!( ( ( gs.flags >> index ) & 1 ) != 0 )) //bit is not set
		return false;
	U64 x = castlingBitboards[index] & allPieces;
	if ( x == 0) return true;
	return false;
}

void getAllMoves( int color, int moves[], int* cntMoves) {
	int cntNonCaps, cntCaps;
	int noncaps[150];
	generateNonCaptures(color, noncaps, &cntNonCaps);
	int caps[75];
	generateCapturesAndPromotions(color, caps, &cntCaps);

	//printf("number of moves: caps %d number of noncaps %d \n",cntCaps,  cntNonCaps);
	int cnt2 = cntNonCaps+ cntCaps;
	if (cnt2==0) {
		*cntMoves=0;
		return;
	}


	int j=0;
	for (int i=0 ; i < cntNonCaps; i++) {
		moves[j]=noncaps[i];
		j++;
	}
	for (int i=0 ; i < cntCaps; i++) {
		moves[j]=caps[i];
		j++;
	}
    *cntMoves=j;
}
void generateCapturesAndPromotions(int color, int moves[], int* cntMoves ) {

	int cnt=0;
	U64 all= gs.bitboard[ALLPIECES];
	assert(all != 0);
	U64 enemyPieces=gs.bitboard[WHITEPIECES+(1-color)];
	assert(enemyPieces != 0);
	cnt = getPawnCapturesAndPromotions(cnt, moves, gs.bitboard[WP + color], all, enemyPieces, color, WP+color, gs.flags, gs.board);

	cnt =getKnightCaptures(cnt,moves, gs.bitboard[WN+color], enemyPieces,WN+color, gs.board);

	cnt =getBishopCaptures(cnt,moves, gs.bitboard[WB+color],all,enemyPieces,WB+color, gs.board);

	cnt = getRookCaptures(cnt,moves, gs.bitboard[WR+color],all,enemyPieces,WR+color, gs.board);

	cnt = getQueenCaptures(cnt,moves, gs.bitboard[WQ+color],all,enemyPieces,WQ+color, gs.board);
	cnt = getKingCaptures( cnt,moves, gs.bitboard[WK+color],enemyPieces,WK+color, gs.board);
	*cntMoves=cnt;
}


void generateNonCaptures(int color, int moves[], int* numMoves ) {     // don't forget to free moves after using
	U64 all= gs.bitboard[ALLPIECES];
    assert(all != 0);

    int cnt=0;
	cnt = getPawnPushes(cnt, moves, gs.bitboard[WP +gs.color], all, gs.color, WP+gs.color);
	cnt = getKnightNonCaptures(cnt,moves,gs.bitboard[WN+gs.color], ~all, WN+gs.color);
	cnt = getBishopNonCaptures(cnt,moves,gs.bitboard[WB+gs.color],all, ~all, WB+gs.color);
	cnt = getRookNonCaptures(cnt,moves,gs.bitboard[WR+gs.color],all, ~all, WR+gs.color);
	cnt = getQueenNonCaptures(cnt,moves,gs.bitboard[WQ+gs.color],all, ~all, WQ+gs.color);
	cnt = getKingNonCaptures(cnt,moves,gs.bitboard[WK+color], ~all,WK+color);

	if (gs.color == BLACK) {
		if (  canCastle(BKSIDE, all))  {
			int move = createMove(BK, 4, 6,0, kcastle,  0)	;
			moves[cnt++]=move;
		}
		if (  canCastle(BQSIDE, all) ) {

			int move = createMove(BK, 4, 2, 0, qcastle,  0)	;
			moves[cnt++]=move;
		}
	}
	else {
		if (  canCastle(WKSIDE, all)) {

			int move = createMove(WK, 60, 62,0, kcastle,  0)	;
			moves[cnt++]=move;
		}
		if (  canCastle(WQSIDE, all)) {

			int move = createMove(WK, 60, 58,0, qcastle,  0)	;
			moves[cnt++]=move;
		}
	}
	*numMoves=cnt;
}


int getPSTValue2(int victim, int to) {

	switch (victim) {
	case WP:
		return  whitePawnTable[to] ;
	case BP:
		return blackPawnTable[to]  ;
	case WN:
		return knightTable[to]  ;
	case BN:
		return blackKnightTable[to] ;
	case WB:
		return whiteBishopTable[to]  ;
	case BB:
		return blackBishopTable[to]  ;
	}
	return 0;
}
int getPSTValue(int pieceMoving, int to, int from) {

	switch (pieceMoving) {
	case WP:
		return  whitePawnTable[to]  -  whitePawnTable[from] ;
	case BP:
		return blackPawnTable[to]  -  blackPawnTable[from] ;
	case WN:
		return knightTable[to]  -  knightTable[from] ;
	case BN:
		return blackKnightTable[to]  -  blackKnightTable[from] ;
	case WB:
		return whiteBishopTable[to]  -  whiteBishopTable[from] ;
	case BB:
		return blackBishopTable[to]  -  blackBishopTable[from] ;
	}
	return 0;
}
int reversePSTValue(int pieceMoving, int to, int from) {

	switch (pieceMoving) {
	case WP:
		return  whitePawnTable[from]  -  whitePawnTable[to] ;
	case BP:
		return blackPawnTable[from]  -  blackPawnTable[to] ;
	case WN:
		return knightTable[from]  -  knightTable[to] ;
	case BN:
		return blackKnightTable[from]  -  blackKnightTable[to] ;
	case WB:
		return whiteBishopTable[from]  -  whiteBishopTable[to] ;
	case BB:
		return blackBishopTable[from]  -  blackBishopTable[to] ;
	}
	return 0;
}
void make(int move) {
	assert ( move != 0);

	int oldCsState[2];
	int newCsState[2];

	oldCsState[WHITE] = gs.flags  & 3;
	oldCsState[BLACK]=  ( gs.flags >> 2 ) & 3;

	gs.seventhRankExtension=false;
	gs.promotion=false;
	gs.moveHistory[gs.moveCounter]= move;
	gs.capturedPieces[gs.moveCounter]= -1;
	int to = ((move >> TO_SHIFT) & SQUARE_MASK);
	int from = move & SQUARE_MASK;
	int myPieceMoving = (move >> PIECE_SHIFT) & PIECE_MASK;
	//int type = (move >> TYPE_SHIFT) & TYPE_MASK;

	U64 fromBB    =   1L <<   ( 63 - from);
	U64  toBB     =   1L <<   ( 63 - to);
	U64 fromToBB = fromBB ^ toBB;

	gs.board[from] = EMPTY;

	int movingPieces=12+gs.color;
	bool resetEP=false;
	int oldEPSquare = gs.flags >>EP_SHIFT;
	int newEPSquare = -1;

	bool resetCastling[2];
	resetCastling[0]=false;
	resetCastling[1]=false;
	int victim;
	int theMoveType = moveType(move);
	if (theMoveType < 1 || theMoveType > 9 ) {
		printf("movetype %d \n", theMoveType);
		printMove(move);
		int myPiecemoving = pieceMoving(move);
		printf("from %d to %d pieceMoving %d\n", from, to, myPiecemoving);

	}
	assert(theMoveType >= 1 && theMoveType <=9);

	if (theMoveType != doublePawnMove) {
		if (oldEPSquare != 0) {
			newEPSquare=0;
			gs.flags = (gs.flags & EP_CLEAR); // Clear the  value
			gs.flags = (gs.flags | (newEPSquare << EP_SHIFT)); // Change the value and return the new irrev state integer
			resetEP=true;
		}
	}
	else {
		const int offset[] = { -8, 8};
		int epSquare2 = from + offset[gs.color];
		newEPSquare = 63 - epSquare2;
		gs.flags = (gs.flags & EP_CLEAR); // Clear the  value
		gs.flags = (gs.flags | (newEPSquare << EP_SHIFT)); // Change the value and return the new irrev state integer
		resetEP=true;
	}
	int promPiece, newto, otherPieces, posValue;;

	switch(theMoveType) {

	case singlePawnMove:
		if ( to >= lowerBound[gs.color] && to <= upperBound[gs.color])
			gs.seventhRankExtension=true;

		posValue =  getPSTValue(myPieceMoving, to, from);
		gs.positional[gs.color] +=posValue;

		gs.board[to] =  myPieceMoving;
		gs.bitboard[myPieceMoving]  ^=  fromToBB;

		gs.bitboard[movingPieces]                   ^=  fromToBB;   // update occupied ...
		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][to];
		break;

	case simple: case doublePawnMove:
		if (myPieceMoving < 6 ) {
			posValue =  getPSTValue(myPieceMoving, to, from);
			gs.positional[gs.color] +=posValue;
		}

		gs.board[to] =  myPieceMoving;
		gs.bitboard[myPieceMoving]  ^=  fromToBB;

		gs.bitboard[movingPieces]                   ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][to];
		break;

	case kcastle:
		gs.positional[gs.color] += CASTLING_BONUS;
		gs.board[to] =  myPieceMoving;
		int rookFrom  = ksFromRook[gs.color];
		int rookTo   =  ksToRook[gs.color];
		gs.board[rookFrom] =  EMPTY;
		gs.board[rookTo] =  castleRook[gs.color];

		gs.bitboard[myPieceMoving]  ^=  fromToBB;

		gs.bitboard[movingPieces]                       ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		U64 rookFromToBB = ksRookFrom[gs.color] ^ ksRookTo[gs.color];

		int theRook = castleRook[gs.color];

		gs.bitboard[theRook]  ^=  rookFromToBB;
		gs.bitboard[12+gs.color]  ^=  rookFromToBB;
		gs.bitboard[ALLPIECES] ^=  rookFromToBB;   // update occupied ...

		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][to];
		gs.hash=  gs.hash ^  gs.pieces[theRook][rookFrom];
		gs.hash=  gs.hash ^  gs.pieces[theRook][rookTo];
		break;

	case qcastle:
		gs.positional[gs.color] += CASTLING_BONUS;
		gs.board[to] =  myPieceMoving;

		rookFrom  = qsFromRook[gs.color];
		rookTo   =  qsToRook[gs.color];

		gs.board[rookFrom] =  EMPTY;
		gs.board[rookTo] =  castleRook[gs.color];

		gs.bitboard[myPieceMoving]  ^=  fromToBB;

		gs.bitboard[movingPieces]                       ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		rookFromToBB = qsRookFrom[gs.color] ^ qsRookTo[gs.color];

		theRook = castleRook[gs.color];
		gs.bitboard[theRook]  ^=  rookFromToBB;
		gs.bitboard[12+gs.color]  ^=  rookFromToBB;
		gs.bitboard[ALLPIECES] ^=  rookFromToBB;   // update occupied ...
		gs.flags = gs.flags & kingSideFlags[gs.color];
		gs.flags = gs.flags & queenSideFlags[gs.color];

		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][to];
		gs.hash=  gs.hash ^  gs.pieces[theRook][rookFrom];
		gs.hash=  gs.hash ^  gs.pieces[theRook][rookTo];
		break;

	case captureNoPromotion:
		victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;
		gs.capturedPieces[gs.moveCounter]= victim;
		int posVal;
		if (myPieceMoving < 6 ) {
			posVal =  getPSTValue(myPieceMoving, to, from);
			gs.positional[gs.color] +=posVal;
		}

		if (victim < 6 )  gs.positional[1-gs.color] -= getPSTValue2(victim,to);

		gs.board[to] =  myPieceMoving;
		gs.bitboard[myPieceMoving]                 ^=  fromToBB;
		gs.bitboard[movingPieces]                ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[victim] 					  ^=  toBB;       // clear bit of the captured piece
		gs.bitboard[12 + (1-gs.color)]             ^=  toBB;       // clear bit of the captured piece
		gs.bitboard[ALLPIECES] = gs.bitboard[WHITEPIECES] | gs.bitboard[BLACKPIECES];

		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][to];
		gs.hash=  gs.hash ^  gs.pieces[victim][to];

		gs.material[ 1-gs.color] -= valueMap[victim];
		break;

	case capturePromotionQueen:

		promPiece = pp[gs.color];

		victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;
		gs.positional[gs.color] -= 50;
		gs.board[to] =  pp[gs.color];
		gs.capturedPieces[gs.moveCounter]= victim;

		U64 notFromBB=~fromBB;

		int myPawn= pawn[gs.color];

		gs.bitboard[myPawn] &=  notFromBB;    //clear bit of myPieceMoving
		gs.bitboard[movingPieces] &=  notFromBB;    //clear bit of myPieces

		gs.bitboard[victim] ^=  toBB;       // toggle bit of the captured piece
		gs.bitboard[12+ (1-gs.color)] ^= toBB;     // toggle bit of his pieces

		gs.bitboard[promPiece] |=  toBB;   // set bit of promotion piece
		gs.bitboard[movingPieces]  |=  toBB;       // set bit of mypieces

		gs.bitboard[ALLPIECES] =   gs.bitboard[WHITEPIECES] |  gs.bitboard[BLACKPIECES];
		if (victim < 6 )  gs.positional[1-gs.color] -= getPSTValue2(victim,to);
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[promPiece][to];
		gs.hash=  gs.hash ^  gs.pieces[victim][to];

		gs.promotion=true;
		gs.material[1-gs.color] -= valueMap[victim];
		gs.material[gs.color] += ( QUEEN_VALUE-PAWN_VALUE);
		break;

	case simplePromotionQueen:
		gs.positional[gs.color] -=50;

		promPiece=pp[gs.color];
		gs.board[to] =  promPiece;

		notFromBB = ~fromBB;
		gs.bitboard[myPieceMoving] &=  notFromBB;    //clear bit
		gs.bitboard[movingPieces] &=  notFromBB;    //clear bit

		gs.bitboard[promPiece] |=   toBB;   // set bit
		gs.bitboard[movingPieces]    |=   toBB;
		gs.bitboard[ALLPIECES]   =  gs.bitboard[WHITEPIECES] | gs.bitboard[BLACKPIECES];

		gs.promotion=true;
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[promPiece][to];

		gs.material[gs.color] += ( QUEEN_VALUE-PAWN_VALUE);
		break;

	case epCapture:
		otherPieces=12 + (1-gs.color);
		newto= to + offset2[gs.color];
		gs.board[newto]= EMPTY;
		gs.board[to] =  myPieceMoving;
		victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;
		gs.positional[1-gs.color] -= getPSTValue2(victim,newto);
		gs.capturedPieces[gs.moveCounter]= victim;
		posVal =  getPSTValue(myPieceMoving, to, from);
		gs.positional[gs.color] +=posVal;
		int idx = 63 -  newto;
		U64 z = ~(1L << idx);
		gs.bitboard[myPieceMoving]  			   ^=  fromToBB;
		gs.bitboard[movingPieces]                ^=  fromToBB;   // update white or black color bitboard

		gs.bitboard[victim]                      &=  z;  // clear bit
		gs.bitboard[otherPieces]                 &=  z;  // clear bit

		gs.bitboard[ALLPIECES]=  gs.bitboard[WHITEPIECES] |  gs.bitboard[BLACKPIECES];

		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][from];
		gs.hash=  gs.hash ^  gs.pieces[myPieceMoving][to];
		gs.hash=  gs.hash ^  gs.pieces[victim][newto];

		gs.material[(1-gs.color)] -= PAWN_VALUE;
		break;
	default :
		printf("Illegal move type in make");
		char s[5];
		moveToString(move,s);
		printf("%s\n",s);
		printf("movetype %d" , moveType(move));
		printf("piecemoving %d ", pieceMoving(move));
		printf("victim %d \n" , capture(move));
		break;
	}
	gs.moveCounter++;
	if (resetEP) {
		gs.hash = gs.hash ^ gs.en_passant[oldEPSquare];
		gs.hash = gs.hash ^ gs.en_passant[newEPSquare];
	}

	for (int i=0; i< 2; i++) {
		if ( gs.board[rookHomeKS[i]] != castleRook[i] ) {
			gs.flags = gs.flags & kingSideFlags[i];
		}
		if ( gs.board[rookHomeQS[i]] != castleRook[i] ) {
			gs.flags = gs.flags & queenSideFlags[i];
		}
	}

	if ( gs.board[kingHome[gs.color]] != king[gs.color]) {
		gs.flags = gs.flags & kingSideFlags[gs.color];
		gs.flags = gs.flags & queenSideFlags[gs.color];
	}
	newCsState[WHITE] = gs.flags  & 3;
	newCsState[BLACK]=  ( gs.flags >> 2 ) & 3;

	if (newCsState[WHITE] != oldCsState[WHITE]) {

		gs.hash =  gs.hash ^ gs.W_CASTLING_RIGHTS[oldCsState[WHITE]];
		gs.hash =  gs.hash ^ gs.W_CASTLING_RIGHTS[newCsState[WHITE]];
	}
	if (newCsState[BLACK] != oldCsState[BLACK]) {
		gs.hash =  gs.hash ^ gs.B_CASTLING_RIGHTS[oldCsState[BLACK]];
		gs.hash =  gs.hash ^ gs.B_CASTLING_RIGHTS[newCsState[BLACK]];

	}
	if (gs.color==BLACK) gs.hash = gs.hash ^  gs.side;

	gs.color=1-gs.color;

}
void unmake(int move, int flags2, U64 hash2) {
	int to = ((move >> TO_SHIFT) & SQUARE_MASK);
	int from = move & SQUARE_MASK;
	int myPieceMoving = (move >> PIECE_SHIFT) & PIECE_MASK;
	int type = (move >> TYPE_SHIFT) & TYPE_MASK;

	U64 fromBB    =   1L <<   (63 -from);
	U64  toBB     =   1L <<   ( 63 - to);
	U64 fromToBB = fromBB ^ toBB;
	gs.moveCounter--;

	gs.board[from] =  myPieceMoving;
	int color2=myPieceMoving %2;
	int posValue, victim, promotionPiece, newto;
	gs.flags=flags2;
	switch(type) {

	case singlePawnMove:
	case simple:
	case doublePawnMove:
		gs.board[to] = EMPTY;

		gs.bitboard[myPieceMoving]  		   ^=  fromToBB;         //put the moving piece back
		gs.bitboard[12+color2]                ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[ALLPIECES]               ^=  fromToBB;   // update occupied ...

		if (myPieceMoving < 6 ) {
			posValue =  reversePSTValue(myPieceMoving, to, from);
			gs.positional[color2] +=posValue;
		}
		break;
	case kcastle:
		gs.positional[color2] -= CASTLING_BONUS;
		gs.board[to] = EMPTY;
		gs.board[ksToRook[color2]] = EMPTY;
		gs.board[ksFromRook[color2]] =  WR+color2;

		gs.bitboard[myPieceMoving]  ^=  fromToBB;         //put the moving piece back
		gs.bitboard[WHITEPIECES+color2]                    ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		U64 rookFromToBB = fook[color2];
		gs.bitboard[WR+color2]  ^=  rookFromToBB;
		gs.bitboard[WHITEPIECES+color2]  ^=  rookFromToBB;
		gs.bitboard[ALLPIECES]    ^=  rookFromToBB;   // update occupied ...
		break;
	case qcastle:
		gs.positional[color2] -= CASTLING_BONUS;
		gs.board[to] = EMPTY;
		gs.board[qsToRook[color2]] = EMPTY;
		gs.board[qsFromRook[color2]] =  WR+color2;
		gs.bitboard[myPieceMoving]  ^=  fromToBB;         //put the moving piece back
		gs.bitboard[WHITEPIECES+color2]                    ^=  fromToBB;   // update white or black color bitboard
		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...
		rookFromToBB = fook2[color2];
		gs.bitboard[WR+color2]  ^=  rookFromToBB;
		gs.bitboard[WHITEPIECES+color2]  ^=  rookFromToBB;
		gs.bitboard[ALLPIECES]    ^=  rookFromToBB;   // update occupied ...
		break;

	case captureNoPromotion:
		victim = capture(move);
		gs.bitboard[myPieceMoving]  ^=  fromToBB;         //put the moving piece back
		victim = capture(move);  // ie; WP or WQ or BP, etc.
		gs.bitboard[victim] ^=  toBB;       // flip bit of the captured piece
		gs.board[to] =  victim;
		int value = valueMap[victim];
		gs.bitboard[12+color2]  ^=  fromToBB;         //put the moving piece back
		gs.bitboard[12 + (1-color2)] ^=  toBB;
		gs.bitboard[ALLPIECES] = gs.bitboard[BLACKPIECES] | gs.bitboard[WHITEPIECES];
		if (myPieceMoving < 6 ) {
			posValue =  reversePSTValue(myPieceMoving, to, from);
			gs.positional[color2] +=posValue;
		}

		if (victim < 6 )  gs.positional[1-color2] += getPSTValue2(victim,to);
		gs.material[1-color2]   += value;
		break;

	case capturePromotionQueen:
		promotionPiece=gs.board[to];
		gs.bitboard[myPieceMoving]  ^=  fromToBB;         //put the moving piece back
		gs.bitboard[12+color2]  ^=  fromToBB;

		gs.bitboard[myPieceMoving]  ^=  toBB;
		gs.bitboard[12+color2]  ^=  toBB;

		victim = capture(move);  // ie; WP or WQ or BP, etc.
		gs.bitboard[victim] ^=  toBB;       // toggle bit of the captured piece
		gs.bitboard[12+(1-color2)] ^=  toBB;       // toggle bit of the captured piece

		gs.board[to] = victim;
		value = valueMap[victim];

		U64 notToBB = ~toBB;
		gs.bitboard[promotionPiece] &= notToBB;    //clear bit
		gs.bitboard[12+color2]   &= notToBB;    //clear bit

		gs.positional[color2] += 50;
		if (victim < 6) gs.positional[1-color2] += getPSTValue2(victim,to);

		gs.bitboard[ALLPIECES] = gs.bitboard[BLACKPIECES] | gs.bitboard[WHITEPIECES];
		gs.material[1-color2] += value;
		gs.material[color2] -=  ( QUEEN_VALUE-PAWN_VALUE);
		break;

	case simplePromotionQueen:
		promotionPiece=gs.board[to];
		gs.board[to] = EMPTY;

		notToBB = ~toBB;

		gs.bitboard[myPieceMoving]    |=    fromBB;     //put the moving piece back
		gs.bitboard[12+color2]      |=    fromBB;
		gs.bitboard[myPieceMoving]    &=    notToBB;
		gs.bitboard[12+color2]      &=    notToBB;
		gs.bitboard[promotionPiece] &=    notToBB;    //clear bit
		gs.bitboard[12+color2]      &=    notToBB;
		gs.bitboard[ALLPIECES] = gs.bitboard[BLACKPIECES] | gs.bitboard[WHITEPIECES];
		gs. material[color2] -=  ( QUEEN_VALUE-PAWN_VALUE);

		gs.positional[color2] +=50;
		break;

	case epCapture:
		newto= to + offset2[color2];
		gs.board[to] = EMPTY;
		int idx;
		gs.bitboard[myPieceMoving]  ^=  fromToBB;
		gs.bitboard[12+color2]  ^=  fromToBB;

		if (myPieceMoving==WP)  {
			gs.board[to+8] = BP;
			idx = 63 -  (to + 8 );
			U64 z = 1L << idx;
			gs.bitboard[BP]  |=  z;
			gs.bitboard[BLACKPIECES]  |=  z;
			gs.positional[BLACK] += getPSTValue2(BP,newto);
			gs.material[BLACK]   += PAWN_VALUE;
		}
		else {
			gs.board[to-8] = WP;

			idx = 63 -  (to  - 8);
			U64 z = 1L << idx;
			gs.bitboard[WP] |= z;
			gs.bitboard[WHITEPIECES]  |=  z;
			gs.positional[WHITE] += getPSTValue2(WP,newto);
			gs.material[WHITE]   += PAWN_VALUE;
		}
		int  posVal =  reversePSTValue(myPieceMoving, to, from);

		gs.positional[color2] +=posVal;
		gs.bitboard[ALLPIECES] = gs.bitboard[BLACKPIECES] | gs.bitboard[WHITEPIECES];

		break;
	}
	gs.color=1-gs.color;

	gs.hash=hash2;
	gs.currentPly--;
}

int  getPositionalValue(int i, int piece) {

	return getPSTValue2(piece,i);
}

int addPositionalValueForType ( int piece) {
	U64 x = gs.bitboard[piece];
	int total=0;

	while (x)
	{
		int idx = bitScanForward(x);
		total += getPositionalValue( 63 - idx, piece);
		x = x & ( x - 1 );
	}
	return total;
}
U64 calcHash()
{
	U64 hash = 0;

	for(int i=0; i < 64 ; i++)
	{
		int piece = gs.board[i];
		if (piece != -1)
			hash ^= gs.pieces[piece][i];
	}
	if( gs.color==BLACK )
		hash = hash ^ gs.side;

	int cState= gs.flags  & 3;

	hash =  hash ^ gs.W_CASTLING_RIGHTS[cState];

	cState= ( gs.flags >>2) & 3;

	hash =  hash ^ gs.B_CASTLING_RIGHTS[cState];


	int epSquare = gs.flags >>EP_SHIFT;
	if (epSquare != 0)
		hash = hash ^ gs.en_passant[epSquare];
	return hash;
}
bool isInCheck( int color) {
	return isInCheckAux(gs.bitboard[WK+color], 1-color);
}

bool isInCheckAux( U64 king, int color) {    //color is the guys who are checking the king

	int idx = bitScanForward(king);

	U64 x = knightMoveArray[idx];
	if ( (x & gs.bitboard[WN + color]) != 0 ) return true;
	if  (( pawnCheckArray[color][idx] & gs.bitboard[WP + color]) != 0 ) return true;

	x = kingMoveArray[idx];
	if ( (x & gs.bitboard[WK + color]) != 0 ) {
		return true;
	}
	U64 queen=gs.bitboard[WQ+color];

	U64 allPieces=gs.bitboard[ALLPIECES];
	x = getBishopAttacks(idx, allPieces);
	if ((x & ( gs.bitboard[WB+color] | queen) )!= 0) return true;

	x = getRookAttacks(idx,allPieces);
	if ((x & ( gs.bitboard[WR+color] | queen))!= 0) return true;

	return false;
}

//to see if a hash move is legal before we dare to play it !
bool isMoveLegal ( int move) {

	// we need to make sure that the squares between from and to, if they exist, are empty
	// if the pieceMoving is a rook, bishop or queen, whether this is a simple of capture move

	int to = ((move >> TO_SHIFT) & SQUARE_MASK);
	if (to < 0 || to > 63) return false;

	int from = move & SQUARE_MASK;
	if (from < 0 || from > 63) return false;

	int myPieceMoving = (move >> PIECE_SHIFT) & PIECE_MASK;
	if (myPieceMoving < 0 || myPieceMoving > 11) return false;

	int victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;
	int moveType2 = moveType(move);
	int color2 = myPieceMoving % 2;
	if (color2 != gs.color) return false;

	int offset[] = { -8, 8};

	if (gs.board[from] != myPieceMoving) return false;
	int * squares=0;
	switch ( myPieceMoving ) {
	case  WB : case BB:
	case  WQ : case BQ :
	case  WR:  case BR:
		squares = getInBetweenSquares(from,to);
		if (squares != 0) {
			int i=0;
			while (squares[i])
			{
				if (gs.board[squares[i]] != EMPTY) {
					//int xx = squares[i];
					//int qq = gs.board[squares[i]];
					//char c = reverseConvertPiece(qq);
					//printf("square %d OCCUPADO PARA %c \n",xx, c);
					return false;
				}
				i++;
			}
			free(squares);
		}
		break;
	};
	int kingside[]= { WKSIDE, BKSIDE};
	int queenside[]= { WQSIDE, BQSIDE};
	int epSquare2;
	switch(moveType2) {
	case (simple) :
			 if (gs.board[to] != EMPTY) return false;
	break;

	case(singlePawnMove) :
			 if (gs.board[to] != EMPTY) return false;
	break;

	case(doublePawnMove) :

			  if (gs.board[to+ offset[color2]] != EMPTY) return false;
	if (gs.board[to] != EMPTY) return false;
	break;

	case (kcastle)  :

			if (! (canCastle( kingside[color2], gs.bitboard[ALLPIECES])  ))  return false;
	break;

	case (qcastle)  :

		 	if (! (canCastle( queenside[color2], gs.bitboard[ALLPIECES])  ))  return false;
	break;

	case(simplePromotionQueen) :
			if (gs.board[to] != EMPTY) return false;
	break;

	case(captureNoPromotion) :
			if (victim < 0 || victim > 9) return false;
	if (gs.board[to] != victim) return false;
	break;

	case ( capturePromotionQueen) :
			 						if (victim < 0 || victim > 9) return false;
	if (gs.board[to] != victim) return false;
	break;

	case ( epCapture) :

		epSquare2 = from + offset[color2];
	if (gs.board[epSquare2] != victim) return false;
	break;
	default: return false;   //weird movetype
	}
	return true;
}
void initializeAll() {
	printf("initializing\n");
	initializeMoveGen();
	generateBishopAttacks();
	generateRookAttacks();
	initializeEval();
	initializeFlags();
	initializeZobrist();
	gs.initialized=true;
}


