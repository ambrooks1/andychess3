andychess3
==========

translation of javalin code into C

Javalin chess engine - Andrew Brooks January 29, 2014

This engine is a hobby project based mostly on stuff I have learned from reading the chess programming wiki : http://chessprogramming.wikispaces.com/

It is now strong enough to beat Fairy Max more often than it loses ( after enduring months of daily beatings from the same (:-) http://home.hccnet.nl/h.g.muller/CVfairy.html It was also able to eke out one draw against the powerful Arasan engine, but will normally lose to it.

This is a rewrite of my original chess project. The first one was all original code, but played horribly weak chess. So I took on a more collaborative approach.

IMPORTANT NOTE

This engine does not have pondering implemented ( yet?). Therefore, if you play it against an engine that does have pondering, it would be like two boxers fighting where one has an arm tied behind his back. ( Pondering means you get to think on an opponts' time, and steal CPU cycles from his search time). So you have to go to the general options menu on winboard/xboard and turn pondering off.


Most of the code is original, but I am reusing code for one very significant and complex piece of functionality from the excellent open-source carballo engine, by Alberto Ruibal : https://github.com/albertoruibal/carballo

I should be clear that most of the ideas used are standard, and typically based on pseudo-code taken from the wiki site.

Much is owed to the Jonatan Pettersson chess blog and open source java engine, mediocre : http://mediocrechess.blogspot.com/

In particular - the ideas for staged move generation, time control, and some code for futility pruning.

Sliding piece ( rook and bishop ) magic bitboard attack generation - copied and pasted from caraballo engine.

SEE - Static Exchange Evaluation : this is completely original in implementation. It is used only in the quiescence search, since the performance penalty ( versus MVV-LVA, which is used in the main part of the search ) is offset by the gains of being able to completely prune losing captures. My check evasion move generation is also pretty original- not based on anyone else’s code. There is a fair amount of originality in the evaluation part, too.

Transposition Table - based on code snippets from the Winboard Forum chess tutorial by Harm Geert Muller ( creator of the Fairy Max/Micro Max engine ) : https://chessprogramming.wikispaces.com/Harm+Geert+Muller Also, the alpha-beta algorithm implementation is based on code snippets from the same tutorial, and also the concept for the quick-and-dirty repetition detection.

Many of the fundamental concepts are adapted from code taken from the Bruce Moreland tutorial ( iterative deepening, aspiration windows, principal variation search). http://web.archive.org/web/20080216031116/http://www.seanet.com/~brucemo/topics/topics.htm

There are numerous pieces of code used in bitboard move generation and evaluation adapted from C code on the wiki site. Many code snippets are owing to Gerd Isenberg.

The pawn move generation is adapted from Peter Ellis Jones : http://peterellisjones.com/post/41238723473/chess-engine-part-ii-move-encoding-and-move-generation

The ideas for the opening book and the winboard/xboard protocol come from the Frittle java open-source engine : http://frittle.sourceforge.net/

The ideas for late move reduction come from the web page about the glaurung engine : http://www.glaurungchess.com/lmr.html

Many of the ideas and techniques come from comments by Robert Hyatt, author of crafty, on the talkchess.com forum : http://www.talkchess.com/forum/index.php http://www.craftychess.com/
