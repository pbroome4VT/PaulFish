#include "pfgame.h"
#include "pfai.h"
#include <stdio.h>
#include <unistd.h>


void initTestGame (Game*g){
	playPlayer(g, 8, 8, GAME_PLAYER_B);
	playPlayer(g, 8, 7, GAME_PLAYER_B);
	playPlayer(g, 8, 6, GAME_PLAYER_B);
	playPlayer(g, 9, 8, GAME_PLAYER_W);
	playPlayer(g, 9, 7, GAME_PLAYER_W);
}

int main(int argc, char *argv[]){
	Game g;
	initGame(&g);
	//initTestGame(&g);
	Player_t player = GAME_PLAYER_B;
	
/*	while(1){
		printFrame(&g);
		char colIn;
		int rowIn;
		printf("player %s enter move : ", PLAYER_STR[player]);
		scanf("%c%d",&colIn, &rowIn);
		while(getchar()!='\n'){}
		int col = colIn-'a';
		int row = rowIn;
		int test = playPlayer(&g, row, col , player);
		if(test == -1){
			printf("GAMEOVER\n");
			return;
		}
		player = getOpp(player);
	}*/

	while(1){
	//	printFullBoard(&g);
		printFrame(&g);
		int test = 0 ;
		while (test == 0){
		char colIn;
		int rowIn;
		printf("player %s enter move : ", PLAYER_STR[player]);
		scanf("%c%d",&colIn, &rowIn);
		while(getchar()!='\n'){}
		int col = colIn-'a';
		int row = rowIn;
		printf("Playing at (%d, %d)\n", col, row);
		test = playPlayer(&g, row, col, player);
		if(test == -1){
			printf("GAME_OVER\n");
			break;
		}
		}

		Eval eval = compute(&g, GAME_PLAYER_W, 4);
		printf("eval = %d, sq = %d\n", eval.eval, eval.sq);
		test = playPlayer(&g, getRow(eval.sq), getCol(eval.sq), GAME_PLAYER_W);
		if(test == -1){
			printf("game over\n");
			break;
		}
	}
	printBoard(&g);
}
