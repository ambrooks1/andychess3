/*
 * magic.h
 *
 *  Created on: Mar 1, 2014
 *      Author: andrewbrooks
 */

#ifndef MAGIC_H_
#define MAGIC_H_

#include "defs.h"

#define b_d  C64(0x00000000000000ff) // down
#define  b_u  C64(0xff00000000000000) // up
#define b_r  C64(0x0101010101010101) // right
#define b_l  C64(0x8080808080808080) // left

void generateBishopAttacks() ;
U64 getBishopAttacks(int index, U64 all) ;

void generateRookAttacks() ;
U64 getRookAttacks(int index, U64 all) ;

#endif /* MAGIC_H_ */
