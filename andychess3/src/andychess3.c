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

struct gameState {
	int 			board[64];
	U64 			bitboard[NUMBITBOARDS];

	int             color;  //player to move
	int 			flags;
	U64 			hash;
	int				material[2];
	int         	positional[2];

	//used for extensions
	bool      promotion;
	bool      seventhRankExtension;
} gs;

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


int main(void) {
	  char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	  parseFen(fen);
	  printBoard();
	  displayBitboards();

	  return EXIT_SUCCESS;
}
