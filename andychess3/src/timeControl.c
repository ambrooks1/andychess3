/*
 * timeControl.c
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */


/*
 * The command to set a conventional time control looks like this:

  level 40 15 0            play 40 moves in 15 minutes, then next 40 in 15, etc.
  level 40 0:30 0

  The command to set an incremental time control looks like this:

  level 0 2 12

	Here the 0 means "play the whole game in this time control period", the 2 means "base=2 minutes", and the 12 means "inc=12 seconds".
	As in conventional clock mode, the second argument to level can be in minutes and seconds.

The number of moves given in the level command (when non-zero) should be taken as the number of moves
 still to do before the specified time will be added to the clock, if the "level" command is received after some moves have already been played.

  The time given should be interpreted as the time left on its clock (including any time left over from the previous sessions),
  and not necessarily the time that will be added to the clock after the specified number of moves has been played.

  This is only relevant in WinBoard 4.3.xx, which might send the engine "level" commands during a game,
   just before the engine has to start thinking about the first move of a new time-control session.

   Example: if at the start of the game "level 40 60 0" was given (40 moves per hour), and the engine receives "level 20 22 0" just before move 41,
    it should understand that it should do the next 20 moves in 22 minutes (pehaps because the secondary session was 20 moves per 15 minutes,
     and it had 7 minutes left on its clock after the first 40 moves).

The command to set an exact number of seconds per move looks like this:

  st 30

This means that each move must be made in at most 30 seconds.
Time not used on one move does not accumulate for use on later moves.
 */

/*  From H.G. Muller on winboard Forum in response to my question:
 *
 * Say you want to bias time usage towards the beginning
 * by allowing moves 20% more than their fair share, you could use

	targetTime = 1.3*timeLeft/(movesLeft+2.9);

	Then at the start of a session, with (say) 40 moves left,
	you would assign 1/33 of the time to the move rather than 1/40.
	While with 1 move left the target time would be 1/3 of the time left, so you could afford a factor 3 overrun.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "defs.h"

extern int movesPerSession;

U64 currentTimeMillisecs() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	//struct timeval tv2;
	//gettimeofday(&tv2, NULL);

	int microseconds = ( tv.tv_sec) * 1000000 + ((int)tv.tv_usec);
	int milliseconds = microseconds/1000;
	/*struct timeval tv3;
	tv3.tv_sec = microseconds/1000000;
	tv3.tv_usec = microseconds%1000000;*/
	return milliseconds;
}

U64 getTimeForThisMove(U64 timeLeft, int movesLeft, U64 increment) {   // from jonatan petterssen

	U64 timeForThisMove = (U64) ((1.1*timeLeft)/ movesLeft + (increment/2));
	//targetTime = 1.3*timeLeft/(movesLeft+2.9);

	// If the increment puts us above the total time left
	// use the timeleft - 0.5 seconds
	if(timeForThisMove >= timeLeft)
		timeForThisMove = timeLeft -500;

	// If 0.5 seconds puts us below 0
	// use 0.1 seconds to atleast get some move.
	if(timeForThisMove <= 0)
		timeForThisMove = 100;

	return (U64)  timeForThisMove;

}
const int GUESSEDLENGTH =40;

void setTimeLevel(char *line) {
	int mps;
	U64 baseTime, increment;

	int sec = 0;
	if (    sscanf(line, "level %d %lld %lld",
			   &mps, &baseTime, &increment)!=3 &&
	        sscanf(line, "level %d %lld:%d %lld",
					&mps, &baseTime, &sec, &increment)!=4)
	{
		printf ("ERROR in level statement\n");
		return;
	}

	// Get moves per session (0 if not tournament mode)
	// Get base time in milliseconds

	// Add seconds component, if any
	if(sec > 0)
	{
		baseTime += sec * 1000;   // convert to milliseconds
	}
	//Search2.baseTime=baseTime;
	// Get move increment from seconds
	//U64 incrementMilliseconds = increment * 1000;
	//Search2.increment=increment;
	// Set clock format on game


	//System.out.println("moves per session " + movesPerSession + " baseTime " + baseTime + " increment " + increment);

	if(mps > 0) // Tournament format
	{
		movesPerSession=mps;
	}

}

