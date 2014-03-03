/*
 * move.h
 *
 *  Created on: Mar 1, 2014
 *      Author: andrewbrooks
 */

#ifndef MOVE_H_
#define  MOVE_H_

#define 				simple	   		       1
#define 				kcastle	    	       2
#define 				qcastle	    	       3
#define 	 			singlePawnMove	       4
#define 	 			doublePawnMove	       5

#define 				simplePromotionQueen   6
#define 				captureNoPromotion	   7
#define 				epCapture			   8
#define 				capturePromotionQueen  9

#define TO_SHIFT  6 // Number of bits to shift to get the to-index
#define PIECE_SHIFT  12 // To get the piece moving
#define CAPTURE_SHIFT  16 // To get the capture
#define TYPE_SHIFT  20 // To the move type
#define ORDERING_SHIFT  25 // To get the ordering value


#define SQUARE_MASK  63 // 6 bits, masks out the rest of the int when it has been shifted so we only get the information we need
#define PIECE_MASK   15 // 4 bits
#define TYPE_MASK  31 // 5 bits
#define ORDERING_CLEAR  0x1FFFFFF //  00000001111111111111111111111111 use with & which clears the ordering value

char *  moveToString(int move);
int createMove(int pieceMoving, int fromIndex, int toIndex, int capture, int type, int ordering);
#endif /* MOVE_H_ */
