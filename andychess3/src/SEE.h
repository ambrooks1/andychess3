/*
 * SEE.h
 *
 *  Created on: Mar 5, 2014
 *      Author: andrewbrooks
 */

#ifndef SEE_H_
#define SEE_H_

int see( MOVE move, int side, int board[]);
int evaluate2(int side, int victim, int attackers[][16],  int ptr[]);

#endif /* SEE_H_ */
