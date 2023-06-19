#include "pfgame.h"
#include <stdio.h>
#include <string.h>


/*==================================================*/

/*==================================================*/



void initGame (Game *g){
	memset(g->board, GAME_EMPTY, sizeof(g->board));
	memset(g->captures, 0 , sizeof(g->captures));
	g->numMoves = 0;
}

void resetGame(Game *g){
	initGame (g);
}

/*
can be used to play peices or erase if player==GAME_EMPTY)
*/
int play(Game *g, char row, char col, Player_t player){	
	if ( isLegalMove( g, row, col ) ) {
		GamePiece pieces[16];
		int numCaps = getMoveCaptures( g, row, col, player, pieces);
		PlayFrame srcFrame = getPlayFrame( g, row, col, player, pieces, numCaps );
		PlayFrame *destFrame = g->playStack + g->numMoves ;
		memcpy(destFrame, &srcFrame, sizeof(PlayFrame));	
		g->board[row][col] = player;
		makeCaptures( g, pieces, numCaps );
		g->numMoves = g->numMoves + 1;
		return 1;
	}
	return 0;
}

PlayFrame getPlayFrame( Game *g, char row, char col, Player_t player, GamePiece pieces[MAX_CAPTURES], char numCaps ){
	PlayFrame playFrame;
	playFrame.row = row;
	playFrame.col = col;
	memcpy( playFrame.oldCaptures, g->captures, sizeof(g->captures) );
	playFrame.numCaps = numCaps;
	memcpy ( playFrame.caps, pieces, sizeof(playFrame.caps) );
	return playFrame;
}

int undo( Game *g ) {
	if ( g->numMoves > 0 ){
		g->numMoves--;
		PlayFrame playFrame = g->playStack[g->numMoves];
		memcpy ( g->captures, playFrame.oldCaptures, sizeof(g->captures) );
		g->board[playFrame.row][playFrame.col] = GAME_EMPTY;
		for( int i = 0; i < playFrame.numCaps; i++ ){
			char pieceRow = playFrame.caps[i].row;
			char pieceCol = playFrame.caps[i].col;
			Player_t player = playFrame.caps[i].player;
			g->board[pieceRow][pieceCol] = player;	
		}
		return 1;
	}
	return 0;
}

int isInBounds(char row, char col){
	return row >=0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

int isLegalMove(Game *g, char row, char col){
	return isInBounds(row,col) && g->board[row][col] == GAME_EMPTY;
}


int isGameOver( Game *g, char row, char col, Player_t player){
	if (isInBounds(row, col)){
		return g->captures[player] >= 10 || isConnect5(g, row, col, player);
	}
	return 0;
}

int isConnect5(Game *g, char row, char col, Player_t player){
	char streak;
	char i;
	// check horizontal
	streak = 1;
	i = 1;
	while(getPlayerAt(g, row, col + (i++)) == player) { streak++; }
	i = 1;
	while(getPlayerAt(g, row, col - (i++)) == player) { streak++; }
	if(streak >= 5) { return 1;}

	// check vertical
	streak = 1;
	i = 1;
	while(getPlayerAt(g, row + (i++), col) == player) { streak++; }
	i = 1;
	while(getPlayerAt(g, row, col - (i++)) == player) { streak++; }
	if(streak >= 5) { return 1;}

	//check minor diag	
	streak = 1;
	i = 1;
	while(getPlayerAt(g, row + i, col + i) == player) {
		streak++;
		i++;
	}
	i = 1;
	while(getPlayerAt(g, row - i, col - i) == player) {
		streak++;
		i++;
	}
	if(streak >= 5) { return 1;}

	// check major diag
	streak = 1;
	i = 1;
	while(getPlayerAt(g, row - i, col + i) == player) {
		streak++;
		i++;
	}
	i = 1;
	while(getPlayerAt(g, row + i, col - i) == player) {
		streak++;
		i++;
	}
	if(streak >= 5) { return 1;}
	return 0;
}


void makeCaptures(Game *g, GamePiece pieces[16], char n){
	for (int i = 0; i < n; i++){ 
		Player_t opp = getOpp(pieces[i].player);
		g->captures[opp]++; // assumes player is GAME_P0 or GAME_P1
		g->board[pieces[i].row][pieces[i].col] = GAME_EMPTY;
	}
}

int getMoveCaptures(Game *g, char row, char col, Player_t player, GamePiece pieces[16]){
	int numCaps = 0;
	if ( isInBounds( row, col ) ){
		Player_t opp = getOpp(player);
		//test all 8 directions for captures
		for (int rowDir = -1; rowDir<=1; rowDir++) {
			for( int colDir = -1; colDir <= 1; colDir++){
				if(	getPlayerAt( g, row + 3 * rowDir, col + 3 * colDir ) == player &&
					getPlayerAt( g, row + 2 * rowDir, col + 2 * colDir ) == opp &&
					getPlayerAt( g, row + 1 * rowDir, col + 1 * colDir ) == opp)
				{
					pieces[numCaps].player = opp;
					pieces[numCaps].row = row + 1 * rowDir;
					pieces[numCaps].col = col + 1 * colDir;
					numCaps++;
					pieces[numCaps].player = opp;
					pieces[numCaps].row = row + 2 * rowDir;
					pieces[numCaps].col = col + 2 * colDir;
					numCaps++;
				}
			}
		}	
	}
	return numCaps;
}

Player_t getOpp(Player_t player){
	if (player == GAME_P0){
		return GAME_P1;
	}else if(player == GAME_P1){
		return GAME_P0;
	}
	return GAME_OUTOFBOUNDS;
}

Player_t getPlayerAt(Game *g, char row, char col){
	if (isInBounds(row, col)){
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

void printGameStats(Game *g){
	printf("numMoves : %d\n", g->numMoves);
	printf("p0 captures : %d\n", g->captures[GAME_P0]);
	printf("p1 captures : %d\n", g->captures[GAME_P1]);
}
