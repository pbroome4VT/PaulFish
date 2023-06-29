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
	printMoves(moves, numBranches);
	if( playerMask & GAME_PB_MASK ){
		//maxmize
		Eval bestBranch = {.eval = -10000, .sq = -1};
		for (int i = 0; i < numBranches; i++){
			play(g, moves[i], GAME_PB_MASK);
			getchar();
			printBoard(g);
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
			getchar();
			printBoard(g);
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

int addIfNonDuplicate(int *arr, int index, int value){
	//linear search to check for duplicate
	for( int i = 0; i < index; i++){
		if(arr[i] == value){
			return 0;
		}
	}
	//if unique move found, add it to list;
	arr[index] = value;
	return 1;
}

/*
	will attempt to add all neighbor non duplicate squares.
*/
int addNonDuplicateNeighbors(int *arr, int arrLen, int startingIndex, Game *g, int sq){	
	int insertIndex = startingIndex;
	//add surrounding 5x5 to found moves
	for(char row = -2; row <= 2; row++){
		char duplicate = 0; // false
		for(char col = -2; col <= 2; col++){
			//array became full, return as if no candidates were able to be added
			if( insertIndex >= arrLen ){
				return startingIndex;
			}
			//array not full
			int candidateSq = sq + pntToSq(row,col);
			//printf("checking (%d,%d)\n", getRow(candidateSq), getCol(candidateSq));
			if (
				(candidateSq >= 0) &&
				(getBoardSq(g, candidateSq) & GAME_IN_BOUNDS_MASK) &&			
				!(getBoardSq(g, candidateSq) & GAME_OCC_FLAG))
			{	
				// candidate is in bounds and not occupied
				//printf("adding if non duplicate\n");
				insertIndex += addIfNonDuplicate(arr, insertIndex, candidateSq);
			}
		}
	}
	// was able to add all neighbors without overflowing array
	// return new of neighbors added
	return insertIndex;
}


//TODO need to test this a lot

/*
will search through previous game move locations and add their neightbors as candidates to search for minimax moves.
*/
int getMoves2(int *arr, int arrLen, Game *g){
	//look at previous moves to find nearby squares of interest
	int movesFound = 0;
	for (int moveIndex = g->numMoves; moveIndex > 0; moveIndex--){
		//attempt to add neighbors of move
		int newMovesFound = addNonDuplicateNeighbors( arr, arrLen, movesFound, g, g->moves[moveIndex] ); 
		if (newMovesFound > movesFound){
			//moves were added
			movesFound = newMovesFound;
		}else{
			break;
		}
	}
	return movesFound;
}

/*
searches along each direction vector. will continue adding oves along direction until it hits two consecutive empty squares or border
*/
int addStarSearchMoves( int *arr, int arrLen, int startingIndex, Game *g, int sq){
	int arrIndex = startingIndex;
	int i;
	char emptyStreak;
	
	//EAST
	i = 1;
	emptyStreak = 0;
	while(arrIndex < arrLen){
		int testSq = sq + i;
		if (
			testSq >= 0 &&				     // in bounds
			(getBoardSq(g,testSq) & GAME_IN_BOUNDS_MASK))   // in game bounds
		{
			if(!(getBoardSq(g, testSq) & GAME_OCC_FLAG)){	
				arrIndex += addIfNonDuplicate(arr, arrIndex, testSq);
				emptyStreak++;
			}else{
				emptyStreak = 0;
			}
		}
		if( emptyStreak == 2 ){
			break;
		}
		i++;
	}
	if( arrIndex >= arrLen ){
		return startingIndex;
	}
	return arrIndex;
}

/*
fetches candiate moves. candidates are found by looking at recent plays and searching horizontall,vertically, and diagonally
*/
int getMoves(int *arr, int arrLen, Game *g){
	int movesFound = 0;
	for(int moveIndex = g->numMoves; moveIndex > 0; moveIndex--){
		int newMovesFound = addStarSearchMoves( arr, arrLen, movesFound, g, g->moves[moveIndex] );
		if(newMovesFound > movesFound){
			movesFound = newMovesFound;	
		}else{
			break;
		}
	}
	return movesFound;
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
	for(int i = 0; i < n; i++){
		printf("(%c,%d) ", 'a' + getCol(moves[i]), getRow(moves[i]));
	}
	printf("\n\n");
}







