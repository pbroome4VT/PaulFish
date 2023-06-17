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


typedef enum PlayerEnum_t Player_t;
enum PlayerEnum_t {GAME_P0, GAME_P1, GAME_EMPTY, GAME_OUTOFBOUNDS};



typedef struct GamePiece Piece;
struct GamePiece
{
	Player_t player;
	char row;
	char col;
};





void initGame (Game *g);

void resetGame(Game *g);

int play(Game *g, char row, char col, Player_t player);

int isGameOver(Game *g, char row, char col, Player_t player);

void makeCaptures( Game *g, Piece pieces[16], int n);

void getCurrGame(Game *g);

int isLegalMove(Game *g, char row, char col);

void printBoard (Game *g);

int isGameOver(Game *g, char row, char col, Player_t player);

int getMoveCaptures( Game *g, char row, char col, Player_t player, Piece pieces[16]);

Player_t getPlayerAt(Game *g, char row, char col);

Player_t getOpp(Player_t player);

int isInBounds(char row, char col);
