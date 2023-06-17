#include "pfgame.h"
#include <stdio.h>
#include <string.h>


/*==================================================*/

void moveCaptures(Game *g, char row, char col, Player_t player, Point p[]);
/*==================================================*/



void initGame (Game *g){
	memset(g->board, GAME_EMPTY, sizeof(g->board));
	memset(g->captures, 0 , sizeof(g->captures));
	g->numMoves = 0;
}

void resetGame(Game *g){
	initGame (g);
}


int play(Game *g, char row, char col, Player_t player){	
	if ( isLegalMove( g, row, col ) ) {
		g->board[row][col] = player;
	}
}

int isLegalMove(Game *g, char row, char col){
	return row>=0 && row<BOARD_SIZE &&
			col>=0 && col<BOARD_SIZE &&
			g->board[row][col] == GAME_EMPTY;
}


int isWinningMove( Game *g, char row, char col, Player_t player){
	if( isLegalMove(g,row,col) ){
		
	}

}

void moveCaptures(Game *g, char row, char col, Player_t player, Point points[16]){
	if ( isLegalMove( g, row, col ) ){
		Player_t opp = getOpp(player);
		//if(	)
	}
}

Player_t getOpp(Player_t player){
	if (player == GAME_P0){
		return GAME_P1;
	}else if(player == GAME_P1){
		return GAME_P0;
	}
	return GAME_OUTOFBOUNDS;
}

Player_t getPieceAt(Game *g, char row, char col){
	if (isLegalMove(g,row,col)){
		return g->board[row][col];
	}else{
		return GAME_OUTOFBOUNDS;
	}
}

void printBoard(Game *g){
	printf("\n");
	for ( char row = BOARD_SIZE - 1; row >=0; row--){
		printf("%2d  ", row);
		for ( char col = 0; col < BOARD_SIZE; col++ ){
			if ( g->board[row][col] == GAME_EMPTY ){
				printf("*");
			}	
			if ( g->board[row][col] == GAME_P0 ){
				printf("0");
			}
			if ( g->board[row][col] == GAME_P1 ){
				printf("1");
			}
			printf(" ");
		}
		printf("\n");
	}
	printf("    ");
	for ( char col = 0; col < BOARD_SIZE; col++ ){
		printf("%c ", 'a'+col);
	}
	printf("\n");
}

