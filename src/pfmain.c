#include "pfgame.h"
#include "pfai.h"
#include <stdio.h>
#include <unistd.h>


void testGame(Game *g){
	play(g, 9, 9, GAME_P0);
	play(g, 8, 9, GAME_P0);
	play(g, 7, 9, GAME_P0);
	play(g, 9, 8, GAME_P1);
	play(g, 8, 8, GAME_P1);
	play(g, 7, 8, GAME_P1);
	play(g, 7, 7, GAME_P1);
}

void undoCmd(Game *g, Player_t *player){
	printf("undo\n");
	undo( g );
	*player = getOpp(*player);
}

int moveCmd(Game *g, Player_t *player){
	char colRead;
	int rowRead;
	scanf("%c%d",&colRead, &rowRead);
	char col = colRead - 'a';
	char row = (char)rowRead;
	int c;
	while ((c=getchar())!='\n'){}
	printf("Playing at (%d, %d)\n", (int)(col), row);
	int x = play( g, row, col, *player );
	if( x == 1 ) {
		*player = getOpp(*player);
		return 1;
	}
	if( x == -1 ){
		printf("cant play there, try again\n");
		return 1;
	}
	return 0;
}

int compCmd(Game *g, Player_t *player){
	int c;
	while ((c=getchar())!='\n'){}
	Eval eval = minimax( NULL, g, 1, *player, 0, 0 );
	printf("eval : %d\n", eval.eval);
	printf("computer playing at (%d,%d)", eval.row, eval.col);
	int x = play( g, eval.row, eval.col, *player );	
	if( x == 1 ) {
		*player = getOpp(*player);
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[]){
	printf("main started\n");
	Game g;
	initGame(&g);
	testGame(&g);
	Player_t player = GAME_P0;
	int keepPlaying = 1;
	printBoard(&g);
	while(keepPlaying){
		printf("player %d enter move: ", player);
		char cmd;
		scanf( "%c", &cmd );
		if (cmd == 'u'){
			undoCmd(&g, &player);
			printBoard(&g);
		}else if (cmd == 'm'){//move
			keepPlaying = moveCmd(&g, &player);
			printBoard(&g);
		}else if (cmd == '\n'){ //make computer move
			keepPlaying = compCmd(&g, &player);
			printBoard(&g);
		}else{
			printf("cmd unknown\n");
		}
	}
	printf("Game OVER");
}



