/*
 * perft.h
 *
 *  Created on: Mar 6, 2014
 *      Author: andrewbrooks
 */

#ifndef PERFT_H_
#define PERFT_H_

void perft_test(int depth);
U64 getNodes() ;
U64 getCaptures() ;
U64 getChecks() ;
U64 getEnPassant();
U64 getCastles() ;
U64 getPromotions() ;
U64 perft2( int depth);
#endif /* PERFT_H_ */
