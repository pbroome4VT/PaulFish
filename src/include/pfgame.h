#ifndef PFGAME_H
#define PFGAME_H





/*=====================================================

		Private variables and functions

=======================================================*/

#define GAME_BOARD_LEN (19)										// board is 19x19
#define GAME_FULL_BOARD_LEN (20)
#define GAME_BOARD_SIZE (400)									// board represented as 20x20 (19x19 with buffer for border)
#define MAX_GAME_MOVES (371)									// max moves is 19x19 + 10 moves of captures


typedef enum PlayerEnum Player_t;
enum PlayerEnum {GAME_PLAYER_W = 0, GAME_PLAYER_B = 1};
static const char *PLAYER_STR[2] = {"white" , "black"};

/* each board location is a char with following bit fields */
#define GAME_PB_MASK (0x01)				// b00000001
#define GAME_PW_MASK (0x02)				// b00000010
#define GAME_IN_BOUNDS_MASK (0x04)		// b00000100
//derived flags

#define GAME_OCC_FLAG (0x03)				// b00000011 
#define GAME_CLEAR_FLAG (0xFC)				// b11111100


#define pntToSq(row, col) ((GAME_FULL_BOARD_LEN * row) + col)		// gets square from (row,col)
#define getRow(sq)	(sq/GAME_FULL_BOARD_LEN)
#define getCol(sq) (sq%GAME_FULL_BOARD_LEN)
#define getPlayer(piece) (piece & GAME_PB_MASK)						//gets player from piece. 1 for black, 0 for white
#define getPlayerMask(player) (player ? GAME_PB_MASK : GAME_PW_MASK)
#define getOpp(player) (player ? GAME_PLAYER_W : GAME_PLAYER_B)
#define getOppMask(piece) (piece ^ GAME_OCC_FLAG)						// gets opposite player mask

typedef struct GameFrameStruct GameFrame;
struct GameFrameStruct{
	char caps[2];			// [1] is black, [0] is white.  5 captures = win
	char board[GAME_BOARD_SIZE];
};


typedef struct GameStruct Game;
struct GameStruct{
	int numMoves;
	int moves[MAX_GAME_MOVES+1];			// index of first move is 1! index 0 is always 0!
	GameFrame frames[MAX_GAME_MOVES+1];		// +1 because there is no 0th move.
};


void initGame (Game *g);


void resetGame(Game *g);


void undo( Game *g);


void play( Game *g, int sq, char playerMask);


char isGameOver(Game*g, int sq);


char getBoardSq(Game *g, int sq);


void makeCaptures( Game *g, int sq);

char isConnect5(Game *g, int sq);

GameFrame *getCurrFrame(Game *g);

/*=====================================================

		Public variables and functions

=======================================================*/




void printFrame(Game *g);


void printBoard(Game *g);


int playPlayer (Game *g, char row, char col, Player_t player);


/*====================================================
			test funcs

===================================================*/
void printBoardMask(Game *g, char mask);

#endif
