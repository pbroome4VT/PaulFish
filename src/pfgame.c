#include "pfgame.h"
#include <stdio.h>
#include <string.h>


/*==================================================*/

/*==================================================*/



void initGame (Game *g){
	memset(g, 0, sizeof(Game));
	//pre-compute the constant inbound sqaures for each frame
	for(int i = 0; i < MAX_GAME_MOVES+1; i++){
		for(int k = 0; k < GAME_BOARD_SIZE; k++){
			if( getRow(k) >= 0 && getRow(k) < GAME_BOARD_LEN && getCol(k) >= 0 && getCol(k) < GAME_BOARD_LEN){
				//is in bounds;
				g->frames[i].board[k] |= GAME_IN_BOUNDS_MASK;
			}
		}
	}

}

void resetGame(Game *g){
	initGame (g);
}

GameFrame *createNewFrame(Game *g){
	g->numMoves++;
	memcpy( (g->frames + g->numMoves), (g->frames + g->numMoves - 1), sizeof (g->frames[0]) );
}


void play( Game *g, int sq, char mask){
	//printf("playing sq %d\n", sq);
	createNewFrame(g);
	g->frames[g->numMoves].board[sq] |= mask;
	g->moves[g->numMoves] = sq;
	makeCaptures(g, sq);
}

void undo( Game *g){
	g->numMoves--;
}

int playPlayer ( Game *g, char row, char col, Player_t player){
	int sq = pntToSq(row, col);
	char mask = 0;
	if(player == GAME_PLAYER_B){
		mask = GAME_PB_MASK;
	}else if (player == GAME_PLAYER_W){
		mask = GAME_PW_MASK;
	}else{
		printf("ERROR unknown player\n");
		return 0;
	}
	if(sq < 0 || sq > GAME_BOARD_SIZE){
		printf("ERROR INVALID SQAURE\n");
		return 0;
	}
	play(g, sq, mask);
	if(isGameOver(g,sq)){	
		return -1;
	}
	return 1;
}

char isGameOver(Game *g, int sq){
	return g->frames[g->numMoves].caps[0] >= 5 || g->frames[g->numMoves].caps[1] >= 5  ||  isConnect5(g, sq);
}

void makeCaptures(Game *g, int sq){
	// check captures in all 8 directions
	char plrMask = getBoardSq(g,sq) & GAME_OCC_FLAG;
	char oppMask = getOppMask(plrMask);
	char *board = g->frames[g->numMoves].board;
	// EAST
	if ((board[sq+1] & oppMask) && 
		(board[sq+2] & oppMask) && 
		(board[sq+3] & plrMask))
	{
		board[sq+1] &= GAME_CLEAR_FLAG;
		board[sq+2] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}

	// NORTH-EAST
	if ((board[sq+21] & oppMask) && 
		(board[sq+42] & oppMask) && 
		(board[sq+63] & plrMask))
	{
		board[sq+21] &= GAME_CLEAR_FLAG;
		board[sq+42] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}
	// NORTH
	if ((board[sq+20] & oppMask) && 
		(board[sq+40] & oppMask) && 
		(board[sq+60] & plrMask))
	{
		board[sq+20] &= GAME_CLEAR_FLAG;
		board[sq+40] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}
	// NORTH-WEST
	if ((board[sq+19] & oppMask) && 
		(board[sq+38] & oppMask) && 
		(board[sq+57] & plrMask))
	{
		board[sq+19] &= GAME_CLEAR_FLAG;
		board[sq+38] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}

	// WEST
	if ((board[(sq-1)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-2)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-3)%GAME_BOARD_SIZE] & plrMask))
	{
		board[sq-1] &= GAME_CLEAR_FLAG;
		board[sq-2] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}

	// SOUTH-WEST
	if ((board[(sq-21)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-42)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-63)%GAME_BOARD_SIZE] & plrMask))
	{
		board[sq-21] &= GAME_CLEAR_FLAG;
		board[sq-42] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}

	// SOUTH
	if ((board[(sq-20)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-40)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-60)%GAME_BOARD_SIZE] & plrMask))
	{
		board[sq-20] &= GAME_CLEAR_FLAG;
		board[sq-40] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}

	// SOUTH-EAST
	if ((board[(sq-19)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-38)%GAME_BOARD_SIZE] & oppMask) && 
		(board[(sq-57)%GAME_BOARD_SIZE] & plrMask))
	{
		board[sq-19] &= GAME_CLEAR_FLAG;
		board[sq-38] &= GAME_CLEAR_FLAG;	
		GameFrame *currFrame = getCurrFrame(g);
		char plr = getPlayer(plrMask);
		((currFrame->caps)[plr])++;
	}
}

char isConnect5(Game *g, int sq){
	//try to count 5 for each dir	
	char plrMask = getBoardSq(g, sq) & GAME_OCC_FLAG;
	char *board = g->frames[g->numMoves].board;
	char streak;

	//horizontal
	streak = 1;
	for(char i = 1; i <= 4 && streak < 5; i++){
		int relativeSq = i;
		if( board[sq+relativeSq] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	for(char i = 1; i <= 4 && streak < 5; i++){
		int relativeSq = -i;
		if( board[(sq+relativeSq)%GAME_BOARD_SIZE] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5 ){
		return 1;
	}

	//major diag
	streak = 1;
	for(char i = 1; i <= 4; i++){
		int relativeSq = i*GAME_FULL_BOARD_LEN + i;
		if( board[sq+relativeSq] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	for(char i = 1; i <= 4; i++){
		int relativeSq = -i*GAME_FULL_BOARD_LEN - i;
		if( board[(sq+relativeSq)%GAME_BOARD_SIZE] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5 ){
		return 1;
	}

	//vertical
	streak = 1;
	for(char i = 1; i <= 4; i++){
		int relativeSq = i * GAME_FULL_BOARD_LEN;
		if( board[sq+relativeSq] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	for(char i = 1; i <= 4; i++){
		int relativeSq = -i * GAME_FULL_BOARD_LEN;
		if( board[(sq+relativeSq)%GAME_BOARD_SIZE] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5 ){
		return 1;
	}

	//minor diag
	streak = 1;
	for(char i = 1; i <= 4; i++){
		int relativeSq = i * GAME_FULL_BOARD_LEN - i;
		if( board[sq+relativeSq] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	for(char i = 1; i <= 4; i++){
		int relativeSq = -i * GAME_FULL_BOARD_LEN + i;
		if( board[(sq+relativeSq)%GAME_BOARD_SIZE] & plrMask ){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5 ){
		return 1;
	}

	return 0;
}

GameFrame *getCurrFrame(Game *g){
	return g->frames + g->numMoves;
}

char getBoardSq(Game *g, int sq){
	return (g->frames[g->numMoves]).board[sq];
}

void printFrame(Game *g){
	printBoard(g);
	char bCaps = g->frames[g->numMoves].caps[1];
	char wCaps = g->frames[g->numMoves].caps[0];
	printf("Black captures:%d\tWhiteCaptures:%d\n", bCaps, wCaps);
	/*printf("moves : ");
	for ( int i = 1; i <= g->numMoves; i ++){
		printf("%d ",g->moves[i]);
	}*/
	printf("\n");
}

void printBoard(Game *g){
	for (char row = GAME_BOARD_LEN-1; row >= 0; row--){
		printf("%2d  ", row);
		for( char col = 0; col < 19; col++){
			int sq = pntToSq(row, col);
			char piece = getBoardSq(g, sq);
			char c = '*';
			if(piece & GAME_PB_MASK){
				c = 'B';
			}else if(piece & GAME_PW_MASK){
				c = 'W';
			}
			printf("%c ", c);
		}
		printf("\n");
	}
	printf("    ");
	for(int col = 0; col < 19; col++){
		printf("%c ", 'a'+col);
	}
	printf("\n");
}

void printBoardMasked(Game *g, char mask){
	for (char row = GAME_BOARD_LEN-1; row >= 0; row--){
		printf("%2d  ", row);
		for( char col = 0; col < 19; col++){
			int sq = pntToSq(row, col);
			char piece = getBoardSq(g, sq);
			if(piece & mask){
				printf("1 ");
			}else{
				printf("0 ");
			}
		}
		printf("\n");
	}
	printf("    ");
	for(int col = 0; col < 19; col++){
		printf("%c ", 'a'+col);
	}
	printf("\n");
}

void printFullBoard(Game *g){
	for(int row = GAME_FULL_BOARD_LEN-1; row >=0; row--){
		for(int col = 0; col < GAME_FULL_BOARD_LEN; col++){
			printf("%d ", getBoardSq(g, pntToSq(row,col)));
		}
		printf("\n");
	}
}
