/*
 * eval.c

 *
 *  Created on: Mar 2, 2014
 *      Author: andrewbrooks
 */
#include <stdlib.h>
#include <stdio.h>
#include "stdbool.h"
#include "defs.h"
#include "util.h"
#include "movegen.h"
#include "pawnStructureTable.h"






const int CENTER_PAWN_BLOCK_PENALTY = -35;
extern gameState gs;

int openingMidgameBonus(int color, int bonus) ;
int endgameBonus(int color, int bonus);
int materialDownBonus(int color, int bonus) ;

bool goodKingsideShield(U64 x, int color) {

	const U64 whiteKSpawnShields[] = { C64(57344), C64(132352), C64(67072), C64(262912),  C64(67109632)  };	 //best, fianchetto, best w/h3, best w/f3, best w/f4
	const U64 blackKSpawnShields[] = { C64(1970324836974592), C64(1409573906808832), C64(1689949371891712),
			C64(848822976643072),  C64(844442110001152)  };	 //best, fianchetto, best w/h3, best w/f3, best w/f4
	if (color==0) {
		for (int i=0; i < 5; i++ )
			if (x == whiteKSpawnShields[i]) return true;
	}
	else {
		for (int i=0; i < 5; i++ )
			if (x == blackKSpawnShields[i]) return true;
	}
	return false;
}

bool goodQueensideShield(U64 x, int color) {
	const U64 whiteQSpawnShields[] = { C64(1792), C64(8413184), C64(4235264), C64(2146304),  C64(536920064)  };	 //best, fianchetto, best w/a3, best w/c3, best w/c4
	const U64 blackQSpawnShields[] = { C64(63050394783186944), C64(45106365017882624), C64(27162335252578304), C64(54078379900534784),  C64(54043332967399424)  };	 //best, fianchetto, best w/a3, best w/c3, best w/c4
	if (color==0) {
		for (int i=0; i < 5; i++ )
			if (x == whiteQSpawnShields[i]) return true;
	}
	else {
		for (int i=0; i < 5; i++ )
			if (x == blackQSpawnShields[i]) return true;
	}
	return false;
}
short queenTable[] =  {
		//queen
		-20,-10,-10, -5, -5,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5,  5,  5,  5,  0,-10,
		-5,  0,  5,  5,  5,  5,  0, -5,
		-5,  0,  5,  5,  5,  5,  0, -5,
		-10,  5,  5,  5,  5,  5,  0,-10,
		-10,  0,  5,  0,  0,  0,  0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20
};
short blackQueenTable[64];


short whitePawnTable[] =
{
		0,  0,  0,  0,  0,  0,  0,  0,
		25, 25, 25, 25, 25, 25, 25, 25,
		22, 22, 22, 22, 22, 22, 22, 22,
		5,  5, 10, 20, 20, 10,  5,  5,
		0,  0,  0, 20, 20,  0,  0,  0,
		5, -5,- 5,  0,  0, -5, -5,  5,
		5, 10, 10,-25,-25, 10, 10,  5,
		0,  0,  0,  0,  0,  0,  0,  0
};

short blackPawnTable[64];


//Knights are encouraged to control the center and stay away from edges to increase mobility:

short knightTable[] =
{
		-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-30,  5, 10,  5,  5, 10,  5,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  5, 15, 20, 20, 15,  5,-30,
		-30,  5, 10,  5,  5, 10,  5,-30,
		-40,-20,  0,  5,  5,  0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50,
};

short blackKnightTable[64];



//Bishops are also encouraged to control the center and stay away from edges and corners:



short whiteBishopTable[] =
{
		-20,-10,-10,-10,-10,-10,-10,-20,
		-10,  0,  0,  0,  0,  0,  0,-10,
		-10,  0,  5, 10, 10,  5,  0,-10,
		-10, -5,  5, 10, 10,  5, -5,-10,
		-10,  0, 10, 10, 10, 10,  0,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10, 10,  0,  0,  0,  0, 10,-10,
		-20,-10,-20,-10,-10,-20,-10,-20,
};

short  blackBishopTable[64];

short kingTableEndGame[] =
{
		-50,-40,-30,-20,-20,-30,-40,-50,
		-20,-20,-10,  0,  0,-10,-20,-20,
		-20,-10, 20, 30, 30, 20,-10,-20,
		-20,-10, 30, 40, 40, 30,-10,-20,
		-20,-10, 30, 40, 40, 30,-10,-20,
		-20,-10, 20, 30, 30, 20,-10,-20,
		-20,-20, -10, 0,  0,-10,-20,-20,
		-50,-40,-30,-20,-20,-30,-40,-50
};

int twoBishopBonus(int color) {
	int myCount =popCount(gs.bitboard[WB+color]);
	int hisCount = popCount(gs.bitboard[WB+(1-color)]);
	if (myCount == 2 && hisCount < 2) {
		return TWO_BISHOPS_BONUS;
	}
	return 0;
}
int  bonus( int color) {

	int bonus=0;

	if (gs.material[WHITE]  + gs.material[BLACK] >= ENDGAME_MATERIAL_VALUE)
	{
		bonus = openingMidgameBonus(color, bonus);

	}
	else {    // specific to endgame
		bonus = endgameBonus(color, bonus);
	}

	if (gs.material[1-color] - gs.material[color] >= 200) // we are down in material at least two pawns
	{
		bonus = materialDownBonus(color, bonus);
	}
	bonus += twoBishopBonus(color);
	const U64 seventhRank[] = { RANK7, RANK2};
	if (( gs.bitboard[WR+color] & seventhRank[color]) != 0) {
		bonus += ROOK_ON_SEVENTH_BONUS;
	}
	//bonus += tropismPenalty(color);

	//System.out.println("bonus " + bonus);

	return bonus;
}

/*
		private int rookEvaluation(int color) {
			int bonus=0;
			U64 rooks=bitboard[WR+color];
			U64 pawns = bitboard[WP+color];

			U64 hash= rooks | pawns;
			if ( rookTable.exists(hash,rooks)) {
				return rookTable.getScore(hash);
			}

			int idx = Long.numberOfTrailingZeros(rooks);
			while (idx < 64)
			{

				bonus = bonus + ROOK_OPEN_OR_HALF_OPEN_FILE_BONUS * BitUtil.isOnOpenOrHalfOpenFile(rooks, pawns);

				rooks = rooks & ( rooks - 1 );
				idx = Long.numberOfTrailingZeros(rooks);
			}
			rookTable.store(hash, rooks, bonus);
			return bonus;
		}*/

int openingMidgameBonus(int color, int bonus) {

	U64 king = gs.bitboard[WK+color];
	U64 pawns = gs.bitboard[WP+color];
	U64 pieces = gs.bitboard[WHITEPIECES+color];

	const U64  ksCastled[]={C64(7),  C64(504403158265495552)};
	const U64  qsCastled[]={C64(224), C64(2305843009213693952)};
	const U64 safeMiddleGameKingSquares[] = {C64(107), C64(7710162562058289152)    };
	const U64 	queenPawnBlocker[] = { C64(1048576), C64(17592186044416) };
	const U64 	 kingPawnBlocker[] = { C64(524288), C64(8796093022208) };
	const U64 	 queenPawns[] = { C64(4096), C64(4503599627370496) };
	const U64 	kingPawns[] =       { C64(2048), C64(2251799813685248) };

	if ( ((queenPawns[color] & pawns) != 0 )
			&&  ((queenPawnBlocker[color] & pieces) != 0 )  )
	{
		bonus += CENTER_PAWN_BLOCK_PENALTY;
	}
	else
		if ( ((kingPawns[color] & pawns) != 0 )
				&&  ((kingPawnBlocker[color] & pieces) != 0 )  )
		{
			bonus += CENTER_PAWN_BLOCK_PENALTY;
		}

	if  (  (  king &  ksCastled[color])    != 0 )
	{

		const U64  wreckedQSPawnShield[] = { C64(3221225472), C64(824633720832)   };
		const U64 kingsideMask[] =  { C64(67569408), C64(1978038598238208)}; // white, black
		U64 x = kingsideMask[color] & pawns;

		if (!goodKingsideShield(x, color))
		{
			//kingside castled with kside pawn shield
			//penalty for moving pawns away from the shield in front of castled king on kingside
			//System.out.println("wingPawnPushPenalty kingside = -35");
			bonus += WING_PAWN_PUSH_PENALTY;
		}
		if ((pawns &   wreckedQSPawnShield[color]) != 0 ) {  // if castled kingside, definitely push QSIDE pawns
			bonus += WING_PAWN_PUSH_BONUS;
		}
	}

	else

		if  (  (  king &  qsCastled[color])    != 0 )
		{
			const U64  wreckedKSPawnShield[] = { C64(50331648), C64(12884901888)   };
			const U64 qsCastlingNudge[] = { C64(64), C64(4611686018427387904) };
			const U64 queensideMask[] = { C64(551608320), C64(63296822826762240)}; // white, black

			if (king &  qsCastlingNudge[color]) bonus += QUEENSIDE_CASTLING_NUDGE;

			U64 x = queensideMask[color] & pawns;

			if (!goodQueensideShield(x, color))
			{
				//penalty for moving pawns away from the shield in front of castled king on kingside
				//System.out.println("wingPawnPushPenalty qside  = -35");
				bonus += WING_PAWN_PUSH_PENALTY;
			}
			if ((pawns &   wreckedKSPawnShield[color]) != 0 ) {  // if castled queensider, definitely push KSIDE pawns
				bonus += WING_PAWN_PUSH_BONUS;
			}
		}

	if (  (king & safeMiddleGameKingSquares[color]) == 0) {  //oops, king took a walk before the endgame
		//System.out.println("kingWalkPenalty = - 35");
		bonus += KING_WALK_PENALTY;
	}
	return bonus;
}

int endgameBonus(int color, int bonus) {
	//endgame bonus for centralized king
	int idx = bitScanForward(gs.bitboard[WK+color]);
	int ktegBonus = kingTableEndGame[idx];
	bonus += ktegBonus;
	//System.out.println("King table endgame bonus " + ktegBonus);
	// encouragement to push pawns in endgame
	U64 pawnRanks[2][2] =
	{{ RANK4, RANK5},
			{ RANK5, RANK4} };
	const int pawnBonus[] = { 20, 30};
	int pawnCountForRank[2];

	for (int i=0; i < 2; i++) {
		pawnCountForRank[i] = popCount(gs.bitboard[WP+color] & pawnRanks[color][i]);
		int egPawnPushBonus = pawnCountForRank[i]*pawnBonus[i];
		//System.out.println("endgame pawn push bonus" + egPawnPushBonus);
		bonus += egPawnPushBonus;
	}
	return bonus;
}

int materialDownBonus(int color, int bonus) {
	//ideas from : http://www.mayothi.com/nagaskakichess4.html

	//encourage drawish stuff:
	//bishops of opposite color

	const U64 	 darkSquares=C64(6172840429334713770);
	const U64 	  lightSquares=~darkSquares;

	int enemyBishopCount = popCount(gs.bitboard[WB+( 1 -color)]);
	int bishopCount = popCount(gs.bitboard[WB+ color]);

	if (bishopCount== 1 && enemyBishopCount== 1) {
		if (( (gs.bitboard[WB+color] & lightSquares) != 0) &&
				( (gs.bitboard[WB+(1-color)] & darkSquares) != 0)) {
			//System.out.println("Bishops of opp color bonus = 50");
			bonus += 50;
		}
		else
			if (( (gs.bitboard[WB+color] & darkSquares) != 0) &&
					( (gs.bitboard[WB+(1-color)] & lightSquares) != 0)) {
				//System.out.println("Bishops of opp color bonus = 50");
				bonus += 50;
			}
	}
	// the fewer the opponents pawns, when we are down,  the better
	int enemyPawnCount = popCount(gs.bitboard[WP+(1-color)]);
	int pawnRemovalBonus =  ( 8 - enemyPawnCount)*PAWN_REMOVAL_BONUS;
	//System.out.println("pawn Removal bonus " + pawnRemovalBonus);
	bonus += pawnRemovalBonus;

	//Give a bonus for two rooks (two rooks can be drawish)
	int rookCount = popCount(gs.bitboard[WR+color]);

	if (rookCount ==2)  {
		//System.out.println("rook count bonus = 20 ");
		bonus += ROOK_COUNT_BONUS;
	}

	//Give a bonus for keeping the queen (the queen tend to make things more difficult for the opponent)
	int queenCount = popCount(gs.bitboard[WQ+color]);
	if (queenCount ==1) {
		//System.out.println("queen count bonus = 20 ");
		bonus += QUEEN_COUNT_BONUS;
	}
	return bonus;
}

void  pawnStructureBonus(int passedPawnBonus[]) {
	//passers are more valuable as the material decreases and as their rank increases

	int bonusByRank[]={ 0, 4, 6, 8, 10, 12, 14, 16 };  // for passed pawns

	U64 whitePawns=gs.bitboard[WP];
	U64 blackPawns=gs.bitboard[BP];

	U64 hash= gs.bitboard[WP] | gs.bitboard[BP];
	if ( pst_exists(hash, whitePawns, blackPawns)) {
		pst_getScore(hash, passedPawnBonus);
		return;
	}
	int totalMaterial= gs.material[0] + gs.material[1];    //4325 is max , below 3000 is endgame

	//PASSED PAWN BONUS
	int x=16;
	if (totalMaterial > 4000) {
		x=8;
	}
	else
		if (totalMaterial > 3000 && totalMaterial <=4000 )  {
			x=10;
		}
		else
			if (totalMaterial > 2000 && totalMaterial <=3000 )  {
				x=12;
			}
			else
				if (totalMaterial > 1000 && totalMaterial <=2000 )  {
					x=14;
				}

	U64 passers[2];
	passers[0]=passers[1]=0;

	passers[0]= passedPawnsWhite(whitePawns, blackPawns);

	passers[1]= passedPawnsBlack(whitePawns, blackPawns);

	int y,ppBonus=0,rank;

	for (int i=0; i < 2; i++) {

		while (passers[i]){
			int idx = bitScanForward(passers[i]);
			rank =  (8 - idx/8);
			if (i == 0) rank = 9-rank;

			y=rank*bonusByRank[rank];
			ppBonus= x+ y;
			passedPawnBonus[i] +=ppBonus;

			passers[i] = passers[i] & ( passers[i] - 1 );
		}
	}
	int bonus[2];
	bonus[0] = passedPawnBonus[0];
	bonus[1] =  passedPawnBonus[1];

	//END PASSED PAWN BONUS

	/*int[] isolated = new int[2];  // number of isolated pawns, white black
			int[] doubled = new int[2];

			isolated[0] = BitUtil.isolatedPawnsWhite(whitePawns, blackPawns);
			bonus[0] += isolated[0]*ISOLATED_PAWN_PENALTY;

			isolated[1] = BitUtil.isolatedPawnsBlack(whitePawns, blackPawns);
			bonus[1] += isolated[1]*ISOLATED_PAWN_PENALTY;

			doubled[0]= BitUtil.doubledPawnsWhite(whitePawns);
			doubled[1]= BitUtil.doubledPawnsBlack(blackPawns);

			int whiterookCount = bitScanForward(bitboard[WR]);
			bonus[0] += doubled[0]*doubledPawnPenalty(whiterookCount);

			int blackrookCount = bitScanForward(bitboard[BR]);
			bonus[1] += doubled[1]*doubledPawnPenalty(blackrookCount);*/

	pst_store(hash, whitePawns, blackPawns, bonus[0], bonus[1]);

}

/*private int  doubledPawnPenalty(int rookCount) {
			switch(rookCount) {
				case 2: return -10;
				case 1: return -15;
				case 0: return -20;
				default : return 0;
			}
		}*/
int getEvaluation() {

	int tot[2];
	int passedPawnBonus[2];
	passedPawnBonus[0]= passedPawnBonus[1]=0;

	pawnStructureBonus(passedPawnBonus);

	tot[0] = gs.material[0]
	        + gs.positional[0]
	        + bonus(0)
	        + passedPawnBonus[0];

	tot[1] = gs.material[1]
	       + gs.positional[1]
	       + bonus(1)
	       + passedPawnBonus[1];
	int eval =  tot[gs.color] - tot[1-gs.color];

/*	printf( "white material   %d: black material   %d \n", gs.material[WHITE], gs.material[BLACK]);
	printf( "white positional %d: black positional %d \n", gs.positional[WHITE], gs.positional[BLACK]);
	printf( "white bonus      %d: black bonus      %d \n", bonus(WHITE),         bonus(BLACK));
	printf( "WHITE PP BONUS   %d:  BLACK PP BONUS  %d \n", passedPawnBonus[WHITE], passedPawnBonus[BLACK]);

	printf( "WHITE total   %d:  BLACK total  %d \n", 		tot[WHITE],		tot[BLACK]);
	printf(" eval %d\n", eval);*/
	return eval;

}
int getEvaluationMaterial() {

	int tot[2];
	tot[0] = gs.material[0];
	tot[1] = gs.material[1] ;

	int eval =  tot[gs.color] - tot[1-gs.color];
	return eval;
}

void initializeEval() {

	for (int i = 0; i < 64; i++) {
		blackQueenTable[i] = queenTable[63-i];
		blackPawnTable[i] = whitePawnTable[63-i];
		blackKnightTable[i] = knightTable[63-i];
		blackBishopTable[i] = whiteBishopTable[63-i];
	}
}
