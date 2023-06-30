#include "pfai.h"
#include <stdio.h>



Eval compute (Game *g, Player_t player, int depth){
	return minimax(g, getPlayerMask(player), depth, -1000, 1000);
}



Eval minimax(Game *g, char playerMask, int depth, int alpha, int beta){	
	if(!depth){
		Eval e = {.eval = 0, .sq=-1};
		return e;
	}
	int moves[MAX_BRANCHES];
	int numBranches = getMoves(moves, MAX_BRANCHES, g);
	//printMoves(moves, numBranches);
	if( playerMask & GAME_PB_MASK ){
		//maxmize
		Eval bestBranch = {.eval = -10000, .sq = -1};
		for (int i = 0; i < numBranches; i++){
			play(g, moves[i], GAME_PB_MASK);
			//getchar();
			//printBoard(g);
			if(isGameOver(g,moves[i])){
				undo(g);
				Eval e = {.eval = 1000, .sq = moves[i]};
				return e;
			}
			Eval testBranch = minimax(g, GAME_PW_MASK, depth-1, alpha, beta);
			if(testBranch.eval > bestBranch.eval){
				bestBranch.eval = testBranch.eval;
				bestBranch.sq = moves[i];
			}
			undo(g);
		}
		return bestBranch;
	}else{
		Eval bestBranch = {.eval = 10000, .sq = -1};
		for (int i = 0; i < numBranches; i++){
			play(g, moves[i], GAME_PW_MASK);
			//getchar();
			//printBoard(g);
			if(isGameOver(g, moves[i])){
				undo(g);
				Eval e = {.eval = -1000, .sq = moves[i]};
				return e;
			}
			Eval testBranch = minimax(g, GAME_PB_MASK, depth-1, alpha, beta);
			if(testBranch.eval < bestBranch.eval){
				bestBranch.eval = testBranch.eval;
				bestBranch.sq = moves[i];
			}
			undo(g);
		}
		return bestBranch;
		//minimize
	}

}




/*
fetches candiate moves. candidates are found by looking at recent plays and searching horizontall,vertically, and diagonally
*/
int getMoves(int *arr, int arrLen, Game *g){
	int totalMovesFound = 0;
	for(int moveIndex = g->numMoves; moveIndex > 0; moveIndex--){	
		int movesFound = getMovesStarSearch( arr, arrLen, totalMovesFound, g, g->moves[moveIndex] );	
		if(movesFound >= 0){
			//found moves
			totalMovesFound += movesFound;	
		}else{
			//arr full, couldnt store moves
			break;
		}
	}
	return totalMovesFound;
}


int getMovesStarSearch( int *arr, int arrLen, int startIndex, Game *g, int sq){
	int arrIndex = startIndex;
	for( char rowDir = -1; rowDir <= 1; rowDir++ ){
		for( char colDir = -1; colDir <= 1; colDir++ ){
			if(colDir == 0 && rowDir == 0) { continue; } //skip 0 vector
			int offset = rowDir * GAME_FULL_BOARD_LEN + colDir;
			int movesAdded = getMovesOffset(arr, arrLen, arrIndex, offset, g, sq);
			if(movesAdded >= 0){
				// star search succeded along offset vector
				arrIndex += movesAdded;
			}else{
				// star search failed along vector: arr out of space
				return -1;
			}
		}
	}
	return arrIndex - startIndex;
}


/*
	looks at game board pieces on integer multiples of offset starting from sq. 
	adds all empty board squares along this offset to array. stops adding if:
	1) arr becomes full 2) adds two offsets in a row
	returns #squares added on success, -1 on failure.
*/
int getMovesOffset( int *arr, int arrLen, int startIndex, int offset, Game *g, int sq){
	int emptySqRun = 0;
	int arrIndex = startIndex;
	int i = 1;
	while (arrIndex < arrLen){	
		int candidateSq = sq + i * offset;
		if( candidateSq > 0   &&   (getBoardSq( g, candidateSq ) & GAME_IN_BOUNDS_MASK) ){	
			//sq in bounds
			if( (getBoardSq( g, candidateSq ) & GAME_OCC_FLAG) == 0 ){
				//empty in-bound sq
				emptySqRun++;
				arrIndex += addIntToSet(arr, arrIndex, candidateSq);
				if( emptySqRun >= 2 ){
					break;
				}
			}else{
				//non-empty in-bound sq
				emptySqRun = 0;
			}
		}else{
			//sq out of bounds
			break;
		}
		i++;
	}
	if ( arrIndex >= arrLen ){
		//move arr overflowed
		return -1;
	}
	return arrIndex - startIndex; 
}

/*
Function inserts value into set assuming value doesnt already exist in indices [0,index)
Returns 1 on success and 0 on failure
*/
int addIntToSet(int *set, int index, int value){
	//linear search to check for duplicate
	for( int i = 0; i < index; i++){
		if(set[i] == value){
			return 0;
		}
	}
	// unique value, add to set;
	set[index] = value;
	return 1;
}
/* ================================================================
					test functions
===================================================================*/

void playMoves(Game *g, int *moves, int n){
	for(int i = 0; i < n; i++){
		play(g, moves[i], GAME_PB_MASK);
	}
}

void printMoves(int *moves, int n){
	printf("Moves : ");
	for(int i = 0; i < n; i++){
		printf("(%c,%d) ", 'a' + getCol(moves[i]), getRow(moves[i]));
	}
	printf("\n\n");
}







