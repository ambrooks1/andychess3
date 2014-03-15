/*
 * book.c
 *
 *  Created on: Mar 10, 2014
 *      Author: andrewbrooks
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include "defs.h"
#include <string.h>
#include "gamestate.h"
#include "util.h"

//public static Map<Long,List<String>> bookMoves = new HashMap<Long, List<String>>();

BookMove bookMoves[NUM_BOOK_MOVES];

extern gameState gs;
void  castlingToString(char castlingStr[5], int *length)
		{

			bool flag=false;
			int k=0;
			for (int i=0; i < 4; i++)
			{
				if (isBitSet(gs.flags,i))
				{
					switch(i) {
						case WKSIDE: castlingStr[k++] = 'K';
						flag=true;
						break;
						case WQSIDE: castlingStr[k++] = 'Q';
						flag=true;
						break;
						case BKSIDE: castlingStr[k++] = 'k';
						flag=true;
						break;
						case BQSIDE: castlingStr[k++] = 'q';
						flag=true;
						break;

					}
				}
			}
			if (!flag)castlingStr[k++] = '-';
			castlingStr[k]= '\0';
			*length=k;
		}



void toFEN( char FEN[], int *fenLen)
{
	int fenIdx=0;
	char board[64];
	for (int i=0; i < 64; i++) {
		board[i]= reverseConvertPiece(gs.board[i]);
	}

	int i,j,k,e;

	for(i=0; i<8; i++)
	{
		e = 0; // No. of empty squares
		for(j=0; j<8; j++)
		{
			k = i*8+j; // Array index
			if(board[k] != '-')
			{
				if( e > 0 )
				{
					char digit = (char)(((int)'0')+e);
					FEN[fenIdx++] = digit;
					e = 0;
				}
				FEN[fenIdx++] = board[k];
			}
			else
			{
				e++;
			}
		}
		if( e > 0 )
		{
			char digit = (char)(((int)'0')+e);
			FEN[fenIdx++] = digit;
		}
		FEN[fenIdx++] ='/';
	}

	fenIdx--;
	FEN[fenIdx++] =' ';

	if (gs.color==WHITE)
		FEN[fenIdx++]='w';
	else
		FEN[fenIdx++]='b';

	FEN[fenIdx++] = ' ';
	int len=0;
	char castlingStr[5];
	castlingToString(castlingStr, &len);
	strcat(FEN, castlingStr);
	fenIdx=fenIdx+len;

	FEN[fenIdx++] = ' ';
	int epSquare = getEPSquare();

	if (epSquare== 0 ) {
		FEN[fenIdx++]='-';
	}
	else {
		const char * epSq = getSquareFromIndex(63 - epSquare);
		FEN[fenIdx++]=epSq[0];
		FEN[fenIdx++]=epSq[1];
	}

	FEN[fenIdx++]='\0';
	*fenLen=fenIdx;
}

int comp (const void * elem1, const void * elem2)
{
	int f = *((int*)elem1);
	int s = *((int*)elem2);
	if (f > s) return  1;
	if (f < s) return -1;
	return 0;
}
void sortBookMoves() {
	qsort (bookMoves, sizeof(bookMoves)/sizeof(*bookMoves), sizeof(*bookMoves), comp);
}
/*
char* randomMove(List<String> moveList) {

		int i = (int) (Math.random() *  ( moveList.size() - 1 ));
		return moveList.get(i);
}*/

void getMove(U64 hash, char moveStr[]) {
	char fen[70];
	int fenLen=0;
	toFEN(fen, &fenLen);


	//U64 key = gs.hash;
	/*
	if (bookMoves.containsKey(key)) {
		List<String> moveList = bookMoves.get(key);
		return randomMove(moveList);
	}
	else {
		return null;
	}*/
}

void createBook(char *fileName)
{
	// input : alternate lines that look like this :

	//rnbqkbnr/ppp1pppp/8/3p4/8/1P6/P1PPPPPP/RNBQKBNR w KQkq d6
	//g8f6{1544} d7d5{506} e7e6{41} f7f5{30} d7d6{10} g7g6{10}

	FILE* file = fopen(fileName, "r");  //should check the result
	if (file == NULL) {
		printf ("File not created okay, errno = %d\n", errno);
		return ;
	}
	char line[256];
	bool isFen=true;
	char* delims = "[{ }]+";

	U64 hash=0;
	while (fgets(line, sizeof(line), file))
	{
		//note that fgets don't strip the terminating \n, checking its
		//  presence would allow to handle lines longer that sizeof(line)
		printf("%s", line);
		if (strcmp(line,"#END#")==0) {
			break;
		}
		if (isFen) {
			parseFen(line);
			hash = gs.hash;
			isFen=false;
		}
		else {
			char *token;
			/* get the first token */
			token = strtok(line, delims);
			/* walk through other tokens */
			while( token != NULL ) {
				printf( "token: %s\n", token );
				token = strtok(NULL, delims);
			}
			isFen=true;
		}
	}
}

