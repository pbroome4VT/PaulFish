#ifndef PFAI_H
#define PFAI_H
#endif

#include "pfgame.h"


typedef struct PaulFishStruct PaulFish;
struct PaulFishStruct{
	int w1;//weights
	int w2;
};


int minimax(PaulFish *pf, Game *game, int depth, int alpha, int beta); 
