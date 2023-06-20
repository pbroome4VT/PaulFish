#ifndef PFAI_H
#define PFAI_H


#include "pfgame.h"


typedef struct Point Point;
struct Point{
	char row;
	char col;
};

typedef struct PaulFishStruct PaulFish;
struct PaulFishStruct{
	int wNumPieces;
};

typedef struct Eval Eval;
struct Eval{
	int eval;
	char row;
	char col;
};

Eval minimax(PaulFish *pf, Game *game, char depth, Player_t player, int alpha, int beta); 

int getMoves(Game *g, Point points[BOARD_SIZE*BOARD_SIZE]);

char pointWithinRad(Game *g, char row, char col, char rad);

int heuristic( Game *g);

char isMaxPlayer(Player_t player);
#endif
