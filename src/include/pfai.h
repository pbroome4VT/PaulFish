#ifndef PFAI_H
#define PFAI_H

#include "pfgame.h"


// Black is "maximizer"

#define MAX_BRANCHES (30)

typedef struct EvalStruct Eval;
struct EvalStruct{
	int eval;
	int sq;
};


Eval compute (Game *g, Player_t player, int depth);

Eval minimax(Game *g, char playerMask, int depth, int alpha, int beta);



int getMoves(int *arr, int n, Game *g);




/*===============================
		test funcs
================================*/

void printMoves(int *moves, int n);
#endif
