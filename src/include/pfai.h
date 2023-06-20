#ifndef PFAI_H
#define PFAI_H

#include "pfgame.h"



/*
GAME_P0 is "maximizing player" and GAME_P1 is "minimizing player"
*/





typedef struct Point Point;
struct Point{
	char row;
	char col;
};



typedef enum BoardStat_t BoardStat_t;
enum BoardStat_t { DIFF_PIECES, DIFF_DOUBLES, DIFF_TRIPLES, DIFF_QUADRUPLES, NUM_STATS };



typedef struct PaulFishStruct PaulFish;
struct PaulFishStruct{
	const char weights[NUM_STATS][NUM_STATS];
};



typedef struct Eval Eval;
struct Eval{
	double eval;
	char row;
	char col;
};



// structure to hold
typedef struct BoardStats BoardStats;
struct BoardStats{
	int stats[NUM_STATS];
};



Eval minimax(PaulFish *pf, Game *game, char depth, Player_t player, int alpha, int beta); 

int getMoves(Game *g, Point points[BOARD_SIZE*BOARD_SIZE]);

char isPieceWithinRad(Game *g, char row, char col, char rad);

int heuristic( Game *g);

char isConnectN(Game *g, char row, char col, const char N, Player_t player);


#endif
