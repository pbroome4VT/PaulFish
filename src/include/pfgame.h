#ifndef PFGAME_H
#define PFGAME_H
#endif


#define BOARD_SIZE 19



typedef struct GameStruct Game;
struct GameStruct
{
	char board[BOARD_SIZE][BOARD_SIZE];
	int numMoves;
	char captures[2];	// captures[GAME_P0] would be # of piece P0 has captured
};


typedef struct GamePoint Point;
struct GamePoint
{
	char row;
	char col;
};




typedef enum PlayerEnum_t Player_t;
enum PlayerEnum_t {GAME_P0, GAME_P1, GAME_EMPTY, GAME_OUTOFBOUNDS};


void initGame (Game *g);

void resetGame(Game *g);

int play(Game *g, char row, char col, Player_t player);

void getCurrGame(Game *g);

int isLegalMove(Game *g, char row, char col);

void printBoard (Game *g);

int isWinningMove(Game *g, char row, char col, Player_t player);

Player_t getPiece(Game *g, char row, char col);

Player_t getOpp(Player_t player);
