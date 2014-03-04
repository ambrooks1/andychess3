#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

int pst_index(U64 hash);

pawnStructureTable pst[PAWN_STRUC_TBL_ENTRIES];

U64 pst_hits=0, pst_stores=0, pst_probes=0;

  void pst_store(U64 pawnHash2,U64 whitePawns2, U64 blackPawns2, int whiteScore2, int blackScore2)
	{
	    pst_stores++;
        int index = pst_index(pawnHash2);

        pst[index].pawnHash=pawnHash2;
        pst[index].whiteScore=whiteScore2;
        pst[index].blackScore=blackScore2;
        pst[index].whitePawns=whitePawns2;
        pst[index].blackPawns=blackPawns2;
	}

   /**
     * Whether or not the entry for the given state exists in the hashtable
     * @param hash  the 64-bit hash of the state
     * @return  <code>true</code> if a valid entry exists in the hashtable, <code>false</code> if not
     */
     bool pst_exists(U64 hash, U64 whitePawns2, U64 blackPawns2)
    {
    	pst_probes++;
    	int index2 = pst_index(hash);

        if ((pst[index2].pawnHash==hash) &&
        		( whitePawns2==pst[index2].whitePawns) && ( blackPawns2==pst[index2].blackPawns)) {
        	pst_hits++;
        	return true;
        }

        return false;
    }

    /**
     * Returns the score for the given state
     * @param hash  the 64-bit hash of the state
     * @return  the score in centipawns
     */
     int* pst_getScore(U64 hash) {
    	 int * retval = (int *) calloc(2, sizeof(int));
         retval[0] = pst[pst_index(hash)].whiteScore;
    	 retval[1] = pst[pst_index(hash)].blackScore;
    	 return retval;
    }


    /**
     * Convert hash code into appropriate index for table by using the modulo function.
     * @param hash      the hash code of the game state
     * @return          the index in the table (array)
     */
     int pst_index(U64 hash)
    {
        return (int)(abs(hash) % PAWN_STRUC_TBL_ENTRIES);
    }

