#ifndef PFGAME_H
#define PFGAME_H


#define BOARD_SIZE 19
#define BOARD_SPACES (BOARD_SIZE*BOARD_SIZE)
#define MAX_GAME_MOVES ((BOARD_SIZE * BOARD_SIZE) + 20)		// mathematically the max number of moves
#define MAX_CAPTURES 16  // mathematically max captures on single move





typedef enum PlayerEnum_t Player_t;
enum PlayerEnum_t {GAME_P0, GAME_P1, GAME_EMPTY, GAME_OUTOFBOUNDS};


typedef struct GamePiece GamePiece;
struct GamePiece{
	Player_t player;
	char row;
	char col;
};



typedef struct PlayFrame PlayFrame;
struct PlayFrame{
	char row;
	char col;
	char oldCaptures[2];
	char numCaps;
	GamePiece caps[MAX_CAPTURES];
};



typedef struct GameStruct Game;
struct GameStruct
{
	char board[BOARD_SIZE][BOARD_SIZE];
	char numMoves;
	char captures[2];	// captures[GAME_P0] would be # of piece P0 has captured
	PlayFrame playStack[MAX_GAME_MOVES];
};




void initGame (Game *g);

void resetGame(Game *g);

int play(Game *g, char row, char col, Player_t player);

int undo( Game *g);

PlayFrame getPlayFrame(Game *g, char row, char col, Player_t, GamePiece captures[MAX_CAPTURES], char numCaptures);

int isGameOver(Game *g, char row, char col, Player_t player);

int isConnect5(Game *g, char row, char col, Player_t player);

void makeCaptures( Game *g, GamePiece pieces[MAX_CAPTURES], char n);

void getCurrGame(Game *g);

int isLegalMove(Game *g, char row, char col);

void printBoard (Game *g);

int isGameOver(Game *g, char row, char col, Player_t player);

int getMoveCaptures( Game *g, char row, char col, Player_t player, GamePiece pieces[MAX_CAPTURES]);

Player_t getPlayerAt(Game *g, char row, char col);

Player_t getOpp(Player_t player);

int isInBounds(char row, char col);

void printGameStats(Game *g);
#endif
