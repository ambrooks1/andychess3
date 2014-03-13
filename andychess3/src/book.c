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

//public static Map<Long,List<String>> bookMoves = new HashMap<Long, List<String>>();

BookMove bookMoves[NUM_BOOK_MOVES];

extern gameState gs;

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

