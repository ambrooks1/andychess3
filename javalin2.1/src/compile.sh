#!/bin/bash
gcc -pg SEE.c gamestate.c	perft.c  SEE_test.c magic.c	perft_test.c TranspositionTable.c make_test.c search.c book.c move.c tester.c engine2.c	movegen.c timeControl.c eval.c	pawnStructureTable.c util.c  -o test_gprof
