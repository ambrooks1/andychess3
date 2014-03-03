/*
 * magic.c
 *
 *  Created on: Mar 1, 2014
 *      Author: andrewbrooks
 */
#include <stdlib.h>
#include "magic.h"
#include "defs.h"

    U64 bishopMask[64];
	U64 *bishopMagic[64];

	U64 rookMask[64];
	U64 *rookMagic[64];

	 const signed char bishopShiftBits[] = {
		  6, 5, 5, 5, 5, 5, 5, 6,
		  5, 5, 5, 5, 5, 5, 5, 5,
		  5, 5, 7, 7, 7, 7, 5, 5,
		  5, 5, 7, 9, 9, 7, 5, 5,
		  5, 5, 7, 9, 9, 7, 5, 5,
		  5, 5, 7, 7, 7, 7, 5, 5,
		  5, 5, 5, 5, 5, 5, 5, 5,
		  6, 5, 5, 5, 5, 5, 5, 6
		};

	 const signed char rookShiftBits[] = {
		  12, 11, 11, 11, 11, 11, 11, 12,
		  11, 10, 10, 10, 10, 10, 10, 11,
		  11, 10, 10, 10, 10, 10, 10, 11,
		  11, 10, 10, 10, 10, 10, 10, 11,
		  11, 10, 10, 10, 10, 10, 10, 11,
		  11, 10, 10, 10, 10, 10, 10, 11,
		  11, 10, 10, 10, 10, 10, 10, 11,
		  12, 11, 11, 11, 11, 11, 11, 12
		};

	 	// Magic numbers generated with MagicNumbersGen
	// there are 64 magic numbers
	 const U64 rookMagicNumber[] =
		{
		C64(0x1080108000400020), C64(0x40200010004000),
		C64(0x100082000441100), C64(0x480041000080080),
		C64(0x100080005000210), C64(0x100020801000400),
		C64(0x280010000800200), C64(0x100008020420100),
		C64(0x400800080400020), C64(0x401000402000),
		C64(0x100801000200080), C64(0x801000800800),
		C64(0x800400080080), C64(0x800200800400),
		C64(0x1000200040100), C64(0x4840800041000080),
		C64(0x20008080004000), C64(0x404010002000),
		C64(0x808010002000), C64(0x828010000800),
		C64(0x808004000800), C64(0x14008002000480),
		C64(0x40002100801), C64(0x20001004084),
		C64(0x802080004000), C64(0x200080400080),
		C64(0x810001080200080), C64(0x10008080080010),
		C64(0x4000080080040080), C64(0x40080020080),
		C64(0x1000100040200), C64(0x80008200004124),
		C64(0x804000800020), C64(0x804000802000),
		C64(0x801000802000), C64(0x2000801000800804),
		C64(0x80080800400), C64(0x80040080800200),
		C64(0x800100800200), C64(0x8042000104),
		C64(0x208040008008), C64(0x10500020004000),
		C64(0x100020008080), C64(0x2000100008008080),
		C64(0x200040008008080), C64(0x8020004008080),
		C64(0x1000200010004), C64(0x100040080420001),
		C64(0x80004000200040), C64(0x200040100140),
		C64(0x20004800100040), C64(0x100080080280),
		C64(0x8100800400080080), C64(0x8004020080040080),
		C64(0x9001000402000100), C64(0x40080410200),
		C64(0x208040110202), C64(0x800810022004012),
		C64(0x1000820004011), C64(0x1002004100009),
		C64(0x41001002480005), C64(0x81000208040001),
		C64(0x4000008201100804), C64(0x2841008402)
		};
const U64 bishopMagicNumber[] =
		{
		C64(0x1020041000484080), C64(0x20204010a0000), C64(0x8020420240000), C64(0x404040085006400),
		C64(0x804242000000108), C64(0x8901008800000), C64(0x1010110400080), C64(0x402401084004),
		C64(0x1000200810208082), C64(0x20802208200), C64(0x4200100102082000), C64(0x1024081040020),
		C64(0x20210000000), C64(0x8210400100), C64(0x10110022000), C64(0x80090088010820),
		C64(0x8001002480800), C64(0x8102082008200), C64(0x41001000408100), C64(0x88000082004000),
		C64(0x204000200940000), C64(0x410201100100), C64(0x2000101012000), C64(0x40201008200c200),
		C64(0x10100004204200), C64(0x2080020010440), C64(0x480004002400), C64(0x2008008008202),
		C64(0x1010080104000), C64(0x1020001004106), C64(0x1040200520800), C64(0x8410000840101),
		C64(0x1201000200400), C64(0x2029000021000), C64(0x4002400080840), C64(0x5000020080080080),
		C64(0x1080200002200), C64(0x4008202028800), C64(0x2080210010080), C64(0x800809200008200),
		C64(0x1082004001000), C64(0x1080202411080), C64(0x840048010101), C64(0x40004010400200),
		C64(0x500811020800400), C64(0x20200040800040), C64(0x1008012800830a00), C64(0x1041102001040),
		C64(0x11010120200000), C64(0x2020222020c00), C64(0x400002402080800), C64(0x20880000),
		C64(0x1122020400), C64(0x11100248084000), C64(0x210111000908000), C64(0x2048102020080),
		C64(0x1000108208024000), C64(0x1004100882000), C64(0x41044100), C64(0x840400),
		C64(0x4208204), C64(0x80000200282020c), C64(0x8a001240100), C64(0x2040104040080)   };

	int magicTransform(U64 b, U64 magic, signed char bits) {
		  return (int)((b * magic) >> (64 - bits));
	}

	 U64 squareAttackedAuxSliderMask(U64 square, int shift, U64 border) {
		U64 ret = 0;
		while ((square & border) == 0) {
			if (shift > 0) square <<= shift; else square >>= -shift;
			if ((square & border) == 0) ret |= square;
		}
		return ret;
	}

	/**
	 * Fills pieces from a mask. Neccesary for magic generation
	 * variable bits is the mask bytes number
	 * index goes from 0 to 2^bits
	 */
	 U64 generatePieces(int index, int bits, U64 mask) {
		  int i;
		  U64 lsb;
		  U64 result = 0L;
		  for (i = 0; i < bits; i++) {
			lsb = mask & (-mask);
		    mask ^= lsb; // Deactivates lsb bit of the mask to get next bit next time
		    if ((index & (1 << i)) != 0) result |= lsb; // if bit is set to 1
		  }
		  return result;
	}

	/**
	 * Attacks for sliding pieces
	 */
	 U64 checkSquareAttackedAux(U64 square, U64 all, int shift, U64 border) {
		U64 ret = 0;
		while ((square & border) == 0) {
			if (shift>0) square <<= shift; else square >>= -shift;
			ret |= square;
			// If we collide with other piece
			if ((square & all) != 0) break;
		}
		return ret;
	}

U64 getBishopShiftAttacks(long square, U64 all) {
		return checkSquareAttackedAux(square, all, +9, b_u | b_l) |
			checkSquareAttackedAux(square, all, +7, b_u | b_r) |
			checkSquareAttackedAux(square, all, -7, b_d | b_l) |
			checkSquareAttackedAux(square, all, -9, b_d | b_r);
	}

U64 getRookShiftAttacks(long square, long all) {
		return checkSquareAttackedAux(square, all, +8, b_u) |
			checkSquareAttackedAux(square, all, -8, b_d) |
			checkSquareAttackedAux(square, all, -1, b_r) |
			checkSquareAttackedAux(square, all, +1, b_l);
	}

void generateRookAttacks() {

		U64 square = 1;
		signed char  i = 0;
		while (square != 0) {

			rookMask[i] = squareAttackedAuxSliderMask(square, +8, b_u)
					| squareAttackedAuxSliderMask(square, -8, b_d)
					| squareAttackedAuxSliderMask(square, -1, b_r)
					| squareAttackedAuxSliderMask(square, +1, b_l);

			// And now generate magics
			int rookPositions = (1 << rookShiftBits[i]);

			rookMagic[i] = (U64 *) calloc(rookPositions, sizeof(U64));

		    for (int j = 0; j < rookPositions; j++) {
		      long pieces = generatePieces(j, rookShiftBits[i], rookMask[i]);
		      int magicIndex = magicTransform(pieces, rookMagicNumber[i], rookShiftBits[i]);
		      rookMagic[i][magicIndex] = getRookShiftAttacks(square, pieces);
		    }

			square <<= 1;
			i++;
		}

	}
void generateBishopAttacks() {

		U64 square = 1;
		signed char i = 0;
		while (square != 0) {

			bishopMask[i] = squareAttackedAuxSliderMask(square, +9, b_u | b_l)
					| squareAttackedAuxSliderMask(square, +7, b_u | b_r)
					| squareAttackedAuxSliderMask(square, -7, b_d | b_l)
					| squareAttackedAuxSliderMask(square, -9, b_d | b_r);

			int bishopPositions = (1 << bishopShiftBits[i]);

			bishopMagic[i] = (U64 *) calloc(bishopPositions, sizeof(U64));

		    for (int j = 0; j < bishopPositions; j++) {
		      long pieces = generatePieces(j, bishopShiftBits[i], bishopMask[i]);
		      int magicIndex = magicTransform(pieces, bishopMagicNumber[i], bishopShiftBits[i]);
		      bishopMagic[i][magicIndex] = getBishopShiftAttacks(square, pieces);
		    }

			square <<= 1;
			i++;
		}
	}

U64 getRookAttacks(int index, U64 all) {
		// You are inputting the square the rook stands on, and the position of all pieces,and getting
		// back all the rook attacks; meaning empty squares and pieces that could be attacked

		int i = magicTransform(all & rookMask[index], rookMagicNumber[index], rookShiftBits[index]);
		return rookMagic[index][i];
	}
U64 getBishopAttacks(int index, U64 all) {
		// You are inputting the square the bishop stands on, and the position of all pieces,and getting
				// back all the rook attacks; meaning empty squares and pieces that could be attacked

		int i = magicTransform(all & bishopMask[index], bishopMagicNumber[index], bishopShiftBits[index]);
		return bishopMagic[index][i];
	}
