/*
 * defs.h
 *
 *  Created on: Feb 26, 2014
 *      Author: andrewbrooks
 */

#ifndef DEFS_H_
#define  DEFS_H_

typedef unsigned long long U64 ;
#define C64(constantU64) constantU64##ULL

#define WHITE 0
#define BLACK 1

#define WP 0
#define BP 1
#define WN 2
#define BN 3
#define WB 4
#define BB 5
#define WR 6
#define BR 7
#define WQ 8
#define BQ 9
#define WK 10
#define BK 11
#define EMPTY -1

#define WHITEPIECES 12
#define BLACKPIECES 13
#define ALLPIECES 14

#define NUMBITBOARDS 15
#define  MAX_PLIES 200

#define  WKSIDE 0
#define  WQSIDE 1
#define  BKSIDE 2
#define  BQSIDE 3

#define   EP_SHIFT  25
#define   EP_CLEAR  0x1FFFFFF

#endif /* DEFS_H_ */
