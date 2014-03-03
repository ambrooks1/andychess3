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
#include "assert.h"
#include "defs.h"

#include "stdbool.h"
#include "util.h"
#include "movegen.h"
#include "move.h"
#include "magic.h"

const U64 XYA =  C64(6);
const U64 XYB =  C64(112);
const U64 XYC =  C64(432345564227567616);
const U64 XYD =  C64(8070450532247928832);

const U64 castlingBitboards[] =  {XYA, XYB, XYC, XYD};
const U64 minedBitboards[2][2] = { { C64(4), C64(16)}, {  C64(288230376151711744),  C64(1152921504606846976)}};
const int castleRook[]={WR,BR};
const int valueMap[] = { PAWN_VALUE, PAWN_VALUE, KNIGHT_VALUE,  KNIGHT_VALUE,
		BISHOP_VALUE, 	BISHOP_VALUE,   ROOK_VALUE, ROOK_VALUE,  QUEEN_VALUE , QUEEN_VALUE};

U64 negWks = ~ (  C64(1)  << WKSIDE);
U64 negWqs = ~ (  C64(1)  << WQSIDE);
U64  negBks = ~ (  C64(1)  << BKSIDE);
U64  negBqs = ~ (  C64(1)  << BQSIDE);

int kingSideFlags[]= {negWks,negBks} ;
const int pp[]={WQ, BQ};
int queenSideFlags[]= {negWqs,negBqs};

const int rookHomeQS[] = { 56, 0 };
const int rookHomeKS[] = { 63, 7};
const int kingHome[] = { 60, 4 };
const int king[] = { WK,BK };

gameState gs;

int* generateCapturesAndPromotions(int color ) ;
int* generateNonCaptures(int color );

//**********************************************************************
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
			printf("piece %d i= %d\n", piece, i);
			gs.bitboard[piece] = setBitLong(gs.bitboard[piece], 63-i );

		}
	}
	gs.bitboard[WHITEPIECES]= gs.bitboard[WP] | gs.bitboard[WN] | gs.bitboard[WB] | gs.bitboard[WR] | gs.bitboard[WQ] | gs.bitboard[WK];
	gs.bitboard[BLACKPIECES]= gs.bitboard[BP] | gs.bitboard[BN] | gs.bitboard[BB] | gs.bitboard[BR] | gs.bitboard[BQ] | gs.bitboard[BK];
	gs.bitboard[ALLPIECES] = gs.bitboard[WHITEPIECES] | gs.bitboard[BLACKPIECES];

	for (int i=0; i < 14; i++) {
		printf("bitboard %d  value %llu\n", i, gs.bitboard[i]);
	}
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
	printf("convertRowsToBoard\n");
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
}
//**********************************************************************


void parseFen( char fen[]) {

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
	 setAllBitboards();
	 printf("Finished parse fen\n");
}

//**********************************************************************

  bool canCastle(int index, long allPieces) {
			/*To check whether castling is legal, we have to check whether
			the squares between the King and the Rook in question, are empty.
			This is easily done by just ANDing a pre- computed
			bitboard of these squares with the occupied bitboard.

			Also, you cannot castle out of check
			*/

			if  (!( ( ( gs.flags >> index ) & 1 ) != 0 )) //bit is not set
				return false;
			long x = castlingBitboards[index] & allPieces;
			if ( x == 0) return true;
			return false;
		}

  int* getAllMoves( int color) {
	  int * noncaps = generateNonCaptures(color);
	  int * caps = generateCapturesAndPromotions(color);

	  int * moves = (int *) calloc(200, sizeof(int));

	  int *ptr=noncaps;
	  int i=0;
	  while (ptr) {
		  moves[i++]= *ptr;
		  ptr++;
	  }
	  ptr=caps;
	  while (ptr) {
	  		  moves[i++]= *ptr;
	  		  ptr++;
	  }
	  free(caps);
	  free(noncaps);
	  return moves;
  }
  int* generateCapturesAndPromotions(int color ) {
	  int * moves = (int *) calloc(75, sizeof(int));
	  int cnt=0;
	  U64 all= gs.bitboard[ALLPIECES];
	  U64 enemyPieces=gs.bitboard[WHITEPIECES+(1-color)];

	  cnt = getPawnCapturesAndPromotions(cnt, moves, gs.bitboard[WP + color], all, enemyPieces, color, WP+color, gs.flags, gs.board);

	  cnt =getKnightCaptures(cnt,moves, gs.bitboard[WN+color], enemyPieces,WN+color, gs.board);

	  cnt =getBishopCaptures(cnt,moves, gs.bitboard[WB+color],all,enemyPieces,WB+color, gs.board);

	  cnt = getRookCaptures(cnt,moves, gs.bitboard[WR+color],all,enemyPieces,WR+color, gs.board);

	  cnt = getQueenCaptures(cnt,moves, gs.bitboard[WQ+color],all,enemyPieces,WQ+color, gs.board);
	  cnt = getKingCaptures( cnt,moves, gs.bitboard[WK+color],enemyPieces,WK+color, gs.board);

	  return moves;
  }


int * generateNonCaptures(int color ) {     // don't forget to free moves after using
		int * moves = (int *) calloc(125, sizeof(int));
		int cnt=0;

		U64 all= gs.bitboard[ALLPIECES];

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

		return moves;
}

void printMoves(int *moves) {

	  int i=0;
	  while (moves[i] != 0) {
		 char *moveStr = moveToString(moves[i]);
		 printf( "%s\n", moveStr);
		 free(moveStr);
		 i++;
	  }
}
void make(int move) {

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
		 int pieceMoving = (move >> PIECE_SHIFT) & PIECE_MASK;
		 //int type = (move >> TYPE_SHIFT) & TYPE_MASK;

		 long fromBB    =   1L <<   ( 63 - from);
		 long  toBB     =   1L <<   ( 63 - to);
		 long fromToBB = fromBB ^ toBB;

		 gs.board[from] = EMPTY;

		 int movingPieces=12+gs.color;
		 bool resetEP=false;
		 int oldEPSquare = gs.flags >>EP_SHIFT;
		 int newEPSquare = -1;

		 bool resetCastling[2];
		 resetCastling[0]=false;
		 resetCastling[1]=false;
		 int victim;
		 int moveType = moveType(move);

		 if (moveType != doublePawnMove) {
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

		 switch(moveType) {

		    case singlePawnMove:
		    	  const int lowerBound = {8, 48};
		    	  const int upperBound = { 15, 55};

		    	if ( to >= lowerBound[gs.color] && to <= upperBound[gs.color])
		    		gs.seventhRankExtension=true;

		    	int posValue =  pieceTables[pieceMoving][to]  -  pieceTables[pieceMoving][from] ;
		    	gs.positional[gs.color] +=posValue;

		    	gs.board[to] =  pieceMoving;
		    	gs.bitboard[pieceMoving]  ^=  fromToBB;

		    	gs.bitboard[movingPieces]                   ^=  fromToBB;   // update occupied ...
		    	gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

				gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[pieceMoving][to];
		    break;

		    case simple: case doublePawnMove:
		    	if (pieceMoving < 6 ) {
		    		posValue =  pieceTables[pieceMoving][to]  - pieceTables[pieceMoving][from] ;
		    		gs.positional[gs.color] +=posValue;
		    	}

		    	gs.board[to] =  pieceMoving;
		    	gs.bitboard[pieceMoving]  ^=  fromToBB;

		    	gs.bitboard[movingPieces]                   ^=  fromToBB;   // update white or black color bitboard
		    	gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		    	gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[pieceMoving][to];
		    	break;

		    case kcastle:
		 		const int ksFromRook[]={63, 7};
		 		const int ksToRook[]  ={61, 5};

		 		const U64 ksRookFrom[]={ C64(1) , C64(72057594037927936) };
		 		const U64 ksRookTo[]={ C64(4) , C64(288230376151711744) };

		 		gs.positional[gs.color] += CASTLING_BONUS;
		 		gs.board[to] =  pieceMoving;

		 		int rookFrom  = ksFromRook[gs.color];
		 		int rookTo   =  ksToRook[gs.color];

		 		gs.board[rookFrom] =  EMPTY;
		 		gs.board[rookTo] =  castleRook[gs.color];

		 		gs.bitboard[pieceMoving]  ^=  fromToBB;

		 		gs.bitboard[movingPieces]                       ^=  fromToBB;   // update white or black color bitboard
		 		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		 		long rookFromToBB = ksRookFrom[gs.color] ^ ksRookTo[gs.color];

		 		int theRook = castleRook[gs.color];

		 		gs.bitboard[theRook]  ^=  rookFromToBB;
		 		gs.bitboard[12+gs.color]  ^=  rookFromToBB;
		 		gs.bitboard[ALLPIECES] ^=  rookFromToBB;   // update occupied ...

		    	gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[pieceMoving][to];

				gs.hash=  gs.hash ^  pieces[theRook][rookFrom];
				gs.hash=  gs.hash ^  pieces[theRook][rookTo];
		 		break;

		    case qcastle:
		 	 	 const int  qsFromRook[]={56, 0};
		 	 	 const int  qsToRook[]  ={59, 3};

		 	 	const U64 qsRookFrom[]={ C64(128) , C64(-9223372036854775808)  };
		 	 	const U64 qsRookTo[]={ C64(16) , C64(1152921504606846976) };

		 		gs.positional[gs.color] += CASTLING_BONUS;
		 		gs.board[to] =  pieceMoving;

		 		rookFrom  = qsFromRook[gs.color];
		 		rookTo   =  qsToRook[gs.color];

		 		gs.board[rookFrom] =  EMPTY;
		 		gs.board[rookTo] =  castleRook[gs.color];

		 		gs.bitboard[pieceMoving]  ^=  fromToBB;

		 		gs.bitboard[movingPieces]                       ^=  fromToBB;   // update white or black color bitboard
		 		gs.bitboard[ALLPIECES]                      ^=  fromToBB;   // update occupied ...

		 		rookFromToBB = qsRookFrom[gs.color] ^ qsRookTo[gs.color];

		 		theRook = castleRook[gs.color];
		 		gs.bitboard[theRook]  ^=  rookFromToBB;
		 		gs.bitboard[12+gs.color]  ^=  rookFromToBB;
		 		gs.bitboard[ALLPIECES] ^=  rookFromToBB;   // update occupied ...

		 		gs.flags = gs.flags & kingSideFlags[gs.color];
		 		gs.flags = gs.flags & queenSideFlags[gs.color];

		    	gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[pieceMoving][to];

				gs.hash=  gs.hash ^  pieces[theRook][rookFrom];
				gs.hash=  gs.hash ^  pieces[theRook][rookTo];
		 		break;


		 	case captureNoPromotion:

		 		victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;

		 		gs.capturedPieces[gs.moveCounter]= victim;

		 		int posVal;
		 		if (pieceMoving < 6 ) {
		 			posVal =  pieceTables[pieceMoving][to]  -  pieceTables[pieceMoving][from] ;
					gs.positional[gs.color] +=posVal;
		 		}

		 		if (victim < 6 )  gs.positional[1-gs.color] -= pieceTables[victim][to];

		 		gs.board[to] =  pieceMoving;
		 		gs.bitboard[pieceMoving]                 ^=  fromToBB;
		 		gs.bitboard[movingPieces]                ^=  fromToBB;   // update white or black color bitboard
		 		gs.bitboard[victim] 					  ^=  toBB;       // clear bit of the captured piece
		 		gs.bitboard[12 + (1-gs.color)]             ^=  toBB;       // clear bit of the captured piece
		 		gs.bitboard[ALLPIECES] = gs.bitboard[WHITEPIECES] | gs.bitboard[BLACKPIECES];

		 		gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[pieceMoving][to];
				gs.hash=  gs.hash ^  pieces[victim][to];

		 		 gs.material[ 1-gs.color] -= valueMap[victim];
		 		 break;

		 	case capturePromotionQueen:

		 		const int pawn[]={WP, BP};

		 		int promotionPiece=pp[gs.color];

		 		victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;
		 		gs.positional[gs.color] -= 50;
		 		gs.board[to] =  pp[gs.color];
				gs.capturedPieces[gs.moveCounter]= victim;

				U64 notFromBB=~fromBB;

				gs.bitboard[pawn[gs.color]] &=  notFromBB;    //clear bit of pieceMoving
				gs.bitboard[movingPieces] &=  notFromBB;    //clear bit of myPieces

				gs.bitboard[victim] ^=  toBB;       // toggle bit of the captured piece
				gs.bitboard[12+ (1-gs.color)] ^= toBB;     // toggle bit of his pieces

				gs.bitboard[promotionPiece] |=  toBB;   // set bit of promotion piece
				gs.bitboard[movingPieces]  |=  toBB;       // set bit of mypieces

				gs.bitboard[ALLPIECES] =   gs.bitboard[WHITEPIECES] |  gs.bitboard[BLACKPIECES];

				if (victim < 6) {
					gs.positional[[(1-gs.color)] -= pieceTables[victim][to];
				}

				gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[promotionPiece][to];
				gs.hash=  gs.hash ^  pieces[victim][to];

				gs.promotion=true;
				gs.material[1-gs.color] -= valueMap[victim];
				gs.material[gs.color] += ( QUEEN_VALUE-PAWN_VALUE);
		 		break;

		 	case simplePromotionQueen:

		 		gs.positional[gs.color] -=50;

		 		promotionPiece=pp[gs.color];
		 		gs.board[to] =  promotionPiece;

		 		notFromBB = ~fromBB;
		 		gs.bitboard[pieceMoving] &=  notFromBB;    //clear bit
		 		gs.bitboard[movingPieces] &=  notFromBB;    //clear bit

		 		gs.bitboard[promotionPiece] |=   toBB;   // set bit
		 		gs.bitboard[movingPieces]    |=   toBB;
		 		gs.bitboard[ALLPIECES]   =  gs.bitboard[WHITEPIECES] | gs.bitboard[BLACKPIECES];

		 		gs.promotion=true;
		 		gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				gs.hash=  gs.hash ^  pieces[promotionPiece][to];

				gs.material[gs.color] += ( QUEEN_VALUE-PAWN_VALUE);
		 		break;

		 	case epCapture:

		 		 const int offset2[]={8, -8};
		 		 int otherPieces=12 + (1-gs.color);
		 		 int newto= to + offset2[gs.color];

				 gs.board[newto]= EMPTY;
				 gs.board[to] =  pieceMoving;

		 		 victim = (move >> CAPTURE_SHIFT) & PIECE_MASK ;
		 		 gs.positional[(1-gs.color)] -= pieceTables[victim][newto];
		 		 gs.capturedPieces[gs.moveCounter]= victim;

		 		 posVal =  pieceTables[pieceMoving][to]  -  pieceTables[pieceMoving][from] ;
				 gs.positional[gs.color] +=posVal;
				 int idx = 63 -  newto;
				 long z = ~(1L << idx);

				 gs.bitboard[pieceMoving]  			   ^=  fromToBB;
				 gs.bitboard[movingPieces]                ^=  fromToBB;   // update white or black color bitboard

				 gs.bitboard[victim]                      &=  z;  // clear bit
				 gs.bitboard[otherPieces]                 &=  z;  // clear bit

				 gs.bitboard[ALLPIECES]=  gs.bitboard[WHITEPIECES] |  gs.bitboard[BLACKPIECES];

				 gs.hash=  gs.hash ^  pieces[pieceMoving][from];
				 gs.hash=  gs.hash ^  pieces[pieceMoving][to];
				 gs.hash=  gs.hash ^  pieces[victim][newto];

				 gs.material[(1-gs.color)] -= PAWN_VALUE;
		 		break;
		 	default :
		 		 printf("Illegal move type in make");

				 printf(toString(move));
				 printf("movetype " + moveType(move));
				 printf("piecemoving " + pieceMoving(move));
				 printf("victim " + capture(move));
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

int main(void) {

	  initializeMoveGen();
	  generateBishopAttacks();
	  generateRookAttacks();
	  initializeEval();

	  //char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
      //char fen[]= "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
      //char fen[] = "rn1qkbnr/ppp2pp1/3p4/4p2p/4P2P/3P3b/PPP2PP1/RNBQKBNR w KQkq - 0 1";
      char fen[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R2qKb1R w KQkq - 0 1";
	  parseFen(fen);
	  printBoard();
	 // displayBitboards();
	  printf("gen non caps");
	  int * moves = generateNonCaptures(gs.color);
	  printf("finished gen non caps");

	  printMoves(moves);
	  free(moves);

	  moves= generateCapturesAndPromotions(gs.color);
	  printMoves(moves);
	  free(moves);

	  return EXIT_SUCCESS;
}
