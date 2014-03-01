/*
 * movegen_defs.h
 *
 *  Created on: Mar 1, 2014
 *      Author: andrewbrooks
 */

#ifndef MOVEGEN_DEFS_H_
#define MOVEGEN_DEFS_H_

#define  RANK8 C64(0xFF00000000000000)

#define  RANK7 C64( 0x00FF000000000000)
#define  RANK6 C64( 0x0000FF0000000000)
#define  RANK5 C64( 0x000000FF00000000)
#define  RANK4 C64( 0x00000000FF000000)
#define  RANK3 C64( 0x0000000000FF0000)
#define  RANK2 C64( 0x000000000000FF00)
#define  RANK1 C64( 0x00000000000000FF)

#define  fileA C64( 0x8080808080808080)
#define  fileB C64( 0x4040404040404040)
#define  fileC C64( 0x2020202020202020)
#define  fileD C64( 0x1010101010101010)
#define  fileE C64( 0x0808080808080808)
#define  fileF C64( 0x0404040404040404)
#define  fileG C64( 0x0202020202020202)
#define  fileH C64( 0x0101010101010101)

#define noVictim 0
#define DEFAULT_SORT_VAL 62

#define TO_SHIFT  6 // Number of bits to shift to get the to-index
#define PIECE_SHIFT  12 // To get the piece moving
#define CAPTURE_SHIFT  16 // To get the capture
#define TYPE_SHIFT  20 // To the move type
#define ORDERING_SHIFT  25 // To get the ordering value


#define SQUARE_MASK  63 // 6 bits, masks out the rest of the int when it has been shifted so we only get the information we need
#define PIECE_MASK   15 // 4 bits
#define TYPE_MASK  31 // 5 bits
#define ORDERING_CLEAR  0x1FFFFFF //  00000001111111111111111111111111 use with & which clears the ordering value

#endif /* MOVEGEN_DEFS_H_ */
