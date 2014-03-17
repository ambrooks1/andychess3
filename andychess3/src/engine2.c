/*
 * engine2.c

 *
 *  Created on: Mar 5, 2014
 *      Author: andrewbrooks
 */
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include "util.h"
#include "movegen.h"
#include "move.h"
#include "magic.h"
#include "eval.h"
#include "gamestate.h"
#include "perft_test.h"
#include "timeControl.h"
#include "search.h"
#include "book.h"
#include "tester.h"

#define  NONE -1
extern gameState gs;
extern bool usingTime;
extern U64 opponentTimeLeft;
extern U64 timeLeft;
extern int usingRepetitionCheck;
extern int maxIterations;

U64 stateHistory[200];  // every new game state hash, actually played, goes in here
int stateHistCtr = 0;    // state HistoryCounter, incremented for every move played on the board
int fiftyMoveCounter=0;

int numBookMovesMade=0;
const char* version = "2.0";

bool outOfBook=true;
extern int createdBook;

//private static Book2 book;
int legalMoves[MAX_MOVES];
int numLegalMoves=0;

bool forceMode=true;

int movesMade=0;

int sideToMove=NONE, computerSide=NONE;    // 0 is WHITE, 1 is BLACK
bool myTurn=false;

bool irreversible(int move) {
	int moveType2 = moveType(move);
	if (moveType2==simple) return false;
	return true;
}

void write(char* message)
{
	printf("%s\n",message);
	fflush(stdout);
}

int toggle(int stm) {
	return ( 1 - stm);
}


int  validate(char moveStr[]) {
	for (int i=0; i < numLegalMoves; i++) {
		int legalMove = legalMoves[i];
		char mvStr[MOVE_STR_SIZE];
		moveToString(legalMove, mvStr);
		if (strcmp(moveStr,mvStr)==0) {
			return legalMove;
		}
	}
	return 0;
}

void displayLegalMoves(int numLegalMoves, int legalMoves[200]) {
	printf("# here are the legal moves : ");

	for (int j = 0; j < numLegalMoves; j++) {
		char mvStr[MOVE_STR_SIZE];
		moveToString(legalMoves[j], mvStr);
		printf("# %d %s\n", j+1, mvStr);
	}
	fflush(stdout);
}

void  applyMove(char moveStr[])
//make the move on our chessboard.
// note that this could be happening right after we did a search OR
// in response to getting a move from the opponent
{
	if (!gs.initialized) return;
	assert( strlen(moveStr) == 4 || strlen(moveStr)== 5);

	if (numLegalMoves < 1) {
		write("Game over; no legal moves");
		return;
	}
	int move = validate(moveStr);
	if (move == 0) {
		printf("Illegal move %s\n", moveStr);
		fflush(stdout);
		displayLegalMoves(numLegalMoves, legalMoves);
		return ;
	}
	if (usingRepetitionCheck) {
		if (irreversible(move)) {
			fiftyMoveCounter=0;
		}
		else {
			fiftyMoveCounter++;
		}
		stateHistory[stateHistCtr++]=gs.hash;
	}

	make(move);

	getLegalMoveList(legalMoves,&numLegalMoves);

	sideToMove = toggle(sideToMove);

}
bool bookMoveNotFound ( char bookMove[]) {
	char foo[MOVE_STR_SIZE-1]=BOOK_MOVE_NOT_FOUND;
	if (strcmp(bookMove, foo)== 0) return true;
	return false;
}
void findAndApplyMove()
//this is called when the GUI sends the opponents move
// so we need to do a search
{
	if (gs.initialized== false) return;

	char bestmove[MOVE_STR_SIZE];
	memset(bestmove,0,sizeof(bestmove));

	if (!outOfBook) {
		char bookMove[MOVE_STR_SIZE-1]=BOOK_MOVE_NOT_FOUND;

		getBookMove(gs.hash, bookMove);
		if ( bookMoveNotFound(bookMove))
		{
			outOfBook=true;
			calcBestMove( bestmove);
		}
		else {
			numBookMovesMade++;
			strcpy(bestmove, bookMove);
		}

	}
	else
		//write("# Doing  calcBestMove");
	calcBestMove( bestmove);
	//write("# Did calcBestMove; doing apply move ");
	applyMove(bestmove);
	//write("# Did  applymove");
	printf("move %s\n" , bestmove);
	fflush(stdout);
	myTurn=false;
}

void newGame() {
	if (gs.initialized== false) return;
	/*
	 *   Reset the board to the standard chess starting position.
	 *   Set White on move.
	 *   Leave force mode and set the engine to play Black.
	 *   Associate the engine's clock with Black and the opponent's clock with White.
	 *   Reset clocks and time controls to the start of a new game.
	 *   Do not ponder on this move, even if pondering is on.
	 *   Remove any search depth limit previously set by the sd command.
	 */
	forceMode=false;

	gs.currentPly=0;
	movesMade=0;
	numBookMovesMade=0;
	stateHistCtr = 0;    // state HistoryCounter, incremented for every move played on the board
	fiftyMoveCounter=0;
	memset(stateHistory,0,sizeof(stateHistory));
	memset(legalMoves,0,sizeof(legalMoves));
	gs.moveCounter=0;

	parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	getLegalMoveList(legalMoves, &numLegalMoves);
	if (createdBook)
	  outOfBook=false;
	else
	  outOfBook=true;
	sideToMove=WHITE;
	computerSide=BLACK;
}

void processMove(char command[COMMAND_LINE_LEN]) {
	if (sideToMove == NONE) {
		//we have received a move for the first time, so the computer must be playing black
		sideToMove = BLACK;
		computerSide = BLACK;
	}
	applyMove(command);
	if (sideToMove == computerSide) {
		myTurn = true;
		findAndApplyMove();
	}
}
void play_chess() {
		char inBuf[COMMAND_LINE_LEN], command[COMMAND_LINE_LEN];
		int i=0;
		initializeAll();
		signal(SIGTERM, SIG_IGN);
		signal(SIGINT, SIG_IGN);

		printf("Welcome to Javalin Chess Engine %s\n", version);
		fflush(stdout);

		write("Type 'new' to start a new game, or 'quit' to end.");
		write("Then input a move ( Ex. 'e2e4' ) .");

		while (1) {
			fflush(stdout);                 // make sure everything is printed before we do something that might take time

			// wait for input, and read it until we have collected a complete line
			for(i = 0; (inBuf[i] = getchar()) != '\n'; i++);
			inBuf[i+1] = 0;

			// extract the first word
			sscanf(inBuf, "%s", command);

			if(!strcmp(command, "quit"))    { break; } // breaks out of infinite loop
			if(!strcmp(command, "force"))   {
				forceMode=true;
				computerSide=NONE;
				continue;
			}
			//if(!strcmp(command, "analyze")) { engineSide = ANALYZE; continue; }
			//if(!strcmp(command, "exit"))    { engineSide = NONE;    continue; }
			if(!strcmp(command, "otim"))    {
				sscanf(inBuf, "otim %llu", &opponentTimeLeft);
				opponentTimeLeft=opponentTimeLeft*10;
				continue;
			}
			if(!strcmp(command, "time"))
			{
				sscanf(inBuf, "time %llu", &timeLeft);
				timeLeft=timeLeft*10;
				continue;
			}

			if(!strcmp(command, "level"))   {
				usingTime=true;
				maxIterations=MAX_DEPTH;
				setTimeLevel(inBuf);
				continue;
			}

			if(!strcmp(command, "protover 2")){
				write("feature setboard=1 analyze=0 variants=\"normal\" colors=0 debug=1 done=1 sigterm=0 sigint=0");
				continue;
			}

			if(!strcmp(command, "sd"))      { sscanf(inBuf, "sd %d", &maxIterations);    continue; }
			if(!strcmp(command, "st"))      {
				//sscanf(inBuf, "st %d", &timePerMove); continue;
				usingTime=true;
				maxIterations=MAX_DEPTH;
			}

			if(!strcmp(command, "ping"))    { printf("pong%s", inBuf+4); continue; }
			//  if(!strcmp(command, ""))        { sscanf(inBuf, " %d", &); continue; }

			if(!strcmp(command, "new"))     {
				newGame();
				continue;
			}

			if(!strcmp(command, "go"))
			{
				if (sideToMove==NONE) {
					sideToMove=WHITE;
				}
				forceMode=false;
				computerSide=sideToMove;
				myTurn=true;
				findAndApplyMove();
				continue;
			}


			if(!strcmp(command, "book"))    {  continue; }
			// ignored commands:
			if(!strcmp(command, "xboard"))  {
				usingTime=true;
				continue;
			}
			if(!strcmp(command, "computer")){ continue; }
			if(!strcmp(command, "name"))    { continue; }
			if(!strcmp(command, "ics"))     { continue; }
			if(!strcmp(command, "accepted")){ continue; }
			if(!strcmp(command, "rejected")){ continue; }
			if(!strcmp(command, "variant")) { continue; }
			if(!strcmp(command, ""))  {  continue; }

			if (isMoveString(command)) {
				processMove(command);
			}
		}
}
int main() {
	//do_all_tests();
	play_chess();
	return 0;
}
