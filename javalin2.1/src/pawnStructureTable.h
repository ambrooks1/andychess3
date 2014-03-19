/*
 * pawnStructureTable.h
 *
 *  Created on: Mar 4, 2014
 *      Author: andrewbrooks
 */

#ifndef PAWNSTRUCTURETABLE_H_
#define PAWNSTRUCTURETABLE_H_

#include <stdbool.h>

void pst_store(U64 pawnHash2,U64 whitePawns2, U64 blackPawns2, int whiteScore2, int blackScore2);
bool pst_exists(U64 hash, U64 whitePawns2, U64 blackPawns2);
int* pst_getScore(U64 hash, int reval[2]);
int pst_index(U64 hash);

#endif /* PAWNSTRUCTURETABLE_H_ */
