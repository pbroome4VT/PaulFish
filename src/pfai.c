#include "pfai.h"
#include <stdio.h>






/* must call with depth > 0 if you want eval to return with a move reccomendation */
Eval minimax(PaulFish *pf, Game *g, char depth, Player_t player, int alpha, int beta){
	Eval eval;
	if( depth ) {
		Point moves[BOARD_SIZE*BOARD_SIZE];
		int numMoves = getMoves(g, moves);
		if ( player == GAME_P0 ){	
			eval.eval = -1;
			for( int n = 0; n < numMoves; n++ ){
				if( play( g, moves[n].row, moves[n].col, player ) == PLAY_GAME_OVER){
					eval.eval = 1;
					eval.row = moves[n].row;
					eval.col = moves[n].col;
					undo(g);
					return eval;
				}else{
					Eval tmpEval = minimax( pf, g, depth-1, getOpp(player), alpha, beta);
					if( tmpEval.eval > eval.eval ){
						eval = tmpEval;
						eval.row = moves[n].row;
						eval.col = moves[n].col;
					}
				}
				undo(g);
			}
		}else{
			eval.eval = 1;
			for( int n = 0; n < numMoves; n++ ){	
				if (play( g, moves[n].row, moves[n].col, player ) == PLAY_GAME_OVER) {
					eval.eval = -1;
					eval.row = moves[n].row;
					eval.col = moves[n].col;
					undo(g);
					return eval;
				}else{
					Eval tmpEval = minimax( pf, g, depth-1, getOpp(player), alpha, beta);
					if( tmpEval.eval < eval.eval ){
						eval = tmpEval;
						eval.row = moves[n].row;
						eval.col = moves[n].col;
					}
				}
				undo(g);
			}
		}
	}
	return eval;
}


int getMoves(Game *g, Point points[BOARD_SIZE*BOARD_SIZE]){
	int n = 0;
	for( char row = 0; row < BOARD_SIZE; row++ ){
		for( char col = 0; col < BOARD_SIZE; col++){
			char radius = 2;
			if( getPlayerAt(g,row,col) == GAME_EMPTY && isPieceWithinRad( g, row, col, radius ) ){
				points[n].row = row;
				points[n].col =col;
				n++;
			}
		}
	}
	if (n == 0){ // first move of game will cause no points to return
		points[0].row = 8;
		points[0].col = 8;
		n++;
	}
	return n;
}


char isPieceWithinRad(Game *g, char row, char col, char rad){
	for(char r = row - rad; r <= row + rad; r++ ){
		for( char c = col - rad; c <= col+rad; c++ ){
			if( isInBounds(r,c) && getPlayerAt(g,r,c) != GAME_EMPTY ){
				return 1;
			}
		}
	}
	return 0;
}


int heuristic( Game *g){
	return 0;
}



char isConnectN(Game  *g, char row, char col, const char N, Player_t player){
	if( getPlayerAt( g, row, col ) == player ){		
		char streak;
		char i;

		// check horizontal
		streak = 1;
		i = 1;
		while(streak < N && getPlayerAt(g, row, col + (i++)) == player) { streak++; }
		i = 1;
		while(streak < N && getPlayerAt(g, row, col - (i++)) == player) { streak++; }
		if(streak >= N) { return 1;}
	
		// check vertical
		streak = 1;
		i = 1;
		while(streak < N && getPlayerAt(g, row + (i++), col) == player) { streak++; }
		i = 1;
		while(streak < N && getPlayerAt(g, row - (i++), col) == player) { streak++; }
		if(streak >= N) { return 1;}
	
		//check minor diag	
		streak = 1;
		i = 1;
		while(streak < N && getPlayerAt(g, row + i, col + i) == player) {
			streak++;
			i++;
		}
		i = 1;
		while(streak < N && getPlayerAt(g, row - i, col - i) == player) {
			streak++;
			i++;
		}
		if(streak >= N) { return 1;}
	
		// check major diag
		streak = 1;
		i = 1;
		while(streak < N && getPlayerAt(g, row - i, col + i) == player) {
			streak++;
			i++;
		}
		i = 1;
		while(streak < N && getPlayerAt(g, row + i, col - i) == player) {
			streak++;
			i++;
		}
		if(streak >= N) { return 1;}
	}
	return 0;
}
