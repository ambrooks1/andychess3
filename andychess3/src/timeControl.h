/*
 * timeControl.h
 *
 *  Created on: Mar 5, 2014
 *      Author: andrewbrooks
 */

#ifndef TIMECONTROL_H_
#define TIMECONTROL_H_

U64 currentTimeMillisecs() ;
U64 getTimeForThisMove(U64 timeLeft, int movesLeft, U64 increment);
void setTimeLevel(char *line);
#endif /* TIMECONTROL_H_ */
