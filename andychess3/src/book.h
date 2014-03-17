/*
 * book.h
 *
 *  Created on: Mar 10, 2014
 *      Author: andrewbrooks
 */

#ifndef BOOK_H_
#define BOOK_H_

void createBook(char *fileName);
void printBookMoves();
void getBookMove(U64 hash, char moveStr[]) ;
void toFEN( char FEN[]);

#endif /* BOOK_H_ */
