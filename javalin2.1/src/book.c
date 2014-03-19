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
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "defs.h"
#include <string.h>
#include "gamestate.h"
#include "util.h"
#include "move.h"

BookMove bookMoves[NUM_BOOK_MOVES];

int lookupBookMove( U64 hash2);
bool createdBook=false;
extern gameState gs;
void  castlingToString(char castlingStr[5])
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
	//printf("in castlingToString; value = %s\n", castlingStr);

}

void toFEN( char FEN[])
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
    FEN[fenIdx]='\0';

	if (gs.color==WHITE)
		strcat(FEN, "w ");
	else
		strcat(FEN, "b ");

	char castlingStr[5];
	castlingToString(castlingStr);
	//printf("castling Str %s\n", castlingStr);
	strcat(FEN, castlingStr);


	strcat(FEN, " ");

	int epSquare = getEPSquare();

	if (epSquare== 0 ) {
		strcat(FEN, "-");
	}
	else {
		const char * epSq = getSquareFromIndex(63 - epSquare);
		strcat(FEN, epSq);
	}
}
int compare(BookMove *elem1, BookMove *elem2)
{
   if ( elem1->hash < elem2->hash)
      return -1;

   else if (elem1->hash > elem2->hash)
      return 1;

   else
      return 0;
}

int compareHashes (U64 hash1, U64 hash2) {
	 if ( hash1 < hash2)
	      return -1;

	   else if (hash1 > hash2)
	      return 1;

	   else
	      return 0;
	}

void sortBookMoves() {
	 qsort((void *) &bookMoves,              // Beginning address of array
	   NUM_BOOK_MOVES,                                 // Number of elements in array
	   sizeof(BookMove),              // Size of each element
	   (compfn)compare );

}

void printBookMoves() {
	for (int i= 0; i < 10; i++) {
			printf("number %d hash %llu  numMoves %d ",
					i+1, bookMoves[i].hash, bookMoves[i].numMoves);
			for (int j=0; j < bookMoves[i].numMoves; j++) {
				 char *p = bookMoves[i].movelist[j];
				printf("%s ", p);
			}
		}
}
void getBookMove(U64 hash, char moveStr[]) {
	char fen[70];

	toFEN(fen);
    parseFen(fen);
	U64 key = gs.hash;
	//printf("trying to find this hash %llu\n", key);


	int index = lookupBookMove(key);
	if (index == -1) {
		//printf("Not found\n");
		return;
	}
    BookMove bookmove = bookMoves[index];
    srand ( time(NULL) );
    int randIndex = rand() % bookmove.numMoves;

	strcpy(moveStr,bookmove.movelist[randIndex]);

}
//binary search
int lookupBookMove( U64 hash2)
{
    int mid, result;
    int high = NUM_BOOK_MOVES - 1;
    int low = 0;

    while ( low <= high )
    {
        mid = (low + high) / 2;
        result = compareHashes(bookMoves[mid].hash, hash2);

        if (result == -1)
            low = mid + 1;
        else if (result == 1)
            high = mid - 1;
        else
            return mid;
    }
    return -1;
}
/*
 * typedef struct BookMove {
	U64 hash;
	char movelist[10][5];  // 10 strings each is 4 characters plus a null
	int numMoves;
}
 */
void createBook(char *fileName)
{
	// input : alternate lines that look like this :

	//rnbqkbnr/ppp1pppp/8/3p4/8/1P6/P1PPPPPP/RNBQKBNR w KQkq d6
	//g8f6{1544} d7d5{506} e7e6{41} f7f5{30} d7d6{10} g7g6{10}

	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("#Current working dir: %s\n", cwd);
		fflush(stdout);
	}
	else {
		printf("#getcwd() error\n");
		fflush(stdout);
	}

	FILE* file = fopen(fileName, "r");  //should check the result
	if (file == NULL) {
		printf ("#Opening book file %s could not be opened for reading %s\n",
				fileName, strerror(errno));
		fflush(stdout);
		return ;
	}
	char line[256];
	bool isFen=true;
	char* delims = "[{ }]+";
	int bookMovesIdx=0;
	U64 hash=0;
	while (fgets(line, sizeof(line), file))
	{
		//note that fgets don't strip the terminating \n, checking its
		//  presence would allow to handle lines longer that sizeof(line)
		//printf("%d %s", bookMovesIdx, line);
		if (strcmp(line,"#END#")==0) {
			break;
		}

		if (isFen) {

			parseFen(line);
			hash = gs.hash;
			bookMoves[bookMovesIdx].hash=hash;
			isFen=false;
		}
		else
		{
			char *token;
			/* get the first token */

			token = strtok(line, delims);
			/* walk through other tokens */
			int k=0;   // k indexes the tokens
			while( token != NULL ) {

				//printf( "token: %s\n", token );
				if (isMoveString(token)) {
					//printf("*** IS a move: %s\n", token);
					for (int i=0; i < 5;i++)
					  bookMoves[bookMovesIdx].movelist[k][i]=token[i];
					  k++;
				}
				else {
					//printf("not a move: %s\n", token);
				}
				token = strtok(NULL, delims);
			}
			bookMoves[bookMovesIdx].numMoves=k;
			bookMovesIdx++;
			isFen=true;

		}
	}
	//printBookMoves();
	sortBookMoves();
	createdBook=true;
}

