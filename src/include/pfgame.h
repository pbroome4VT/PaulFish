#ifndef PFGAME_H
#define PFGAME_H

#include <stdint.h>

#define GAME_MAX_MOVES (371)

typedef enum PlayerEnum player_t;
enum PlayerEnum {Black = 0;

typedef struct GameFrameStruct GameFrame;
struct GameFrameStruct{
	char captures [2];
	uint64_t horzBB[7];
	uint64_t vertBB[7];
	uint64_t diag1BB[7]; /* /  */
	uint64_t diag2BB[7] /*  \  */
	uint64_t horzChebyshev1BB[7];		
	uint64_t horzChebyshev2BB[7];
};

typedef struct GameStruct Game;
struct GameStruct{
	
};



/*=====================================================

		Private variables and functions

=======================================================*/





#endif
