#include <pfgame.h>
#include <stdio.h>
#include <unistd.h>


void testGame(Game *g){
	play(g, 9, 9, GAME_P0);
	play(g, 8, 9, GAME_P0);
	play(g, 7, 9, GAME_P0);
	play(g, 9, 8, GAME_P1);
	play(g, 8, 8, GAME_P1);
	play(g, 7, 8, GAME_P1);
	play(g, 7,7,GAME_P1);
}
int main(int argc, char *argv[]){
	printf("main started\n");
	Game g;
	initGame(&g);
	testGame(&g);
	Player_t player = GAME_P0;
	while(1){	
		printBoard(&g);
		printf("player %d enter move : ", player);
		int n;
		unsigned char col;
		unsigned int row;
		scanf("%c%d", &col, &row);
		int c;
		while ((c=getchar())!='\n'){}
		printf("Playing at (%d, %d)\n", (int)(col-'a'), row);
		if(!play(&g, (char)row, col-'a', player)){
			printf("cannot play there, try again\n");
			continue;
		}
		if(player == GAME_P0){
			player = GAME_P1;
		}else{
			player = GAME_P0;
		}
	}
}

