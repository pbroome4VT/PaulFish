#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define GAME_MAX_MOVES (371)


/*======================================
			Game types
========================================*/


typedef enum PlayerEnum Player_t;
enum PlayerEnum {BLACK = 0, WHITE = 1};



typedef struct BitboardStruct Bitboard;
struct BitboardStruct{
	uint64_t bbChunks[7];
};



typedef struct GameStruct Game;
struct GameStruct{
	// state variables
	char captures [2];		/* all bitboard indexed by Player_t. example horzBB[BLACK] */
	Bitboard rankBb[2];
	Bitboard filesBb[2];
	Bitboard diag1Bb[2]; /* /  */
	Bitboard diag2Bb[2]; /*  \  */
	Bitboard rankChebyshev1Bb;		
	Bitboard rankChebyshev2Bb;
};





/*======================================
		Precomputed Lookup Tables
========================================*/

/* conversions from [rank][file] points to bitboard coordinates of chunks and indices */
int pntToRankBbChunk[20][20];
int pntToRankBbIndex[20][20];

int pntToFileBbChunk[20][20];
int pntToFileBbIndex[20][20];

int pntToDiag1BbChunk[20][20];
int pntToDiag1BbIndex[20][20];

int pntToDiag2BbChunk[20][20];
int pntToDiag2BbIndex[20][20];


/* conversions from [chunk][index] bitboard coordinates to ranks and files */
int rankBbPntToRank[7][64];
int rankBbPntToFile[7][64];

int fileBbPntToRank[7][64];
int fileBbpntToFile[7][64];

int diag1BbPntToRank[7][64];
int diag1BbPntToFile[7][64];

int diag2BbPntToRank[7][64];
int diag2BbPntToFile[7][64];

/* precomputed rank (chunk,index) to rankChebychev1Bb */
//Bitboard rankBbPntToChebyshevBb[7][64];




/*======================================
		    Helper Functions
========================================*/		

#define setBit(bbChunk, index)		(bbChunk |= 1ULL<<index)
#define getBit(bbChunk, index)		(bbChunk &= 1ULL<<index)
#define popBit(bbChunk, index)		(getBit(bbChunk, index) ? bbChunk ^= (1ULL<<index) : 0)
int getLS1B(uint64_t bb);



/*===================================
			Global Vars
====================================*/
 int g_numMoves;
 Game g_game[GAME_MAX_MOVES+1];



/*==================================

			Init Functions

===================================*/

void initPntToRankBbChunkTable(){
	for (int rank = 0; rank <= 19; rank++){
		for (int file = 0; file <= 19; file++){
			pntToRankBbChunk[rank][file] =  rank / 3;
		}
	}
}

void initPntToFileBbChunkTable(){
	for (int rank = 0; rank <= 19; rank++){
		for (int file = 0; file <= 19; file++){
			pntToFileBbChunk[rank][file] =  file / 3;
		}
	}
}



void initPntToDiag1BbChunkTable(){
	int chunk = 0;
	int cnt = 0;
	int diagStartRank;
	int diagStartFile;

	//count upper triangle chunks
	diagStartFile = 0;
	for (diagStartRank = 19; diagStartRank >= 0; diagStartRank--){;
		int diagLen = 20 - diagStartRank;
		if (cnt + diagLen >= 64 ){
			// diag will cause overflow, must add diag to next chunk
			chunk++;
			cnt = 0;
		}
		//count along diagonal and add them to chunk
		for(int i = 0; i < diagLen; i++){
			int rank = diagStartRank + i;
			int file = diagStartFile + i;
			pntToDiag1BbChunk[rank][file] = chunk;
		}
		cnt += diagLen;
	}

	//count lower triange chunks
	diagStartRank = 0;
	for (diagStartFile = 1; diagStartFile <= 19; diagStartFile++){
		int diagLen = 20 - diagStartFile;	
		if (cnt + diagLen >= 64 ){
			// diag will cause overflow, must add diag to next chunk
			chunk++;
			cnt = 0;
		}
		//count along diagonal and add them to chunk
		for(int i = 0; i < diagLen; i++){
			int rank = diagStartRank + i;
			int file = diagStartFile + i;
			pntToDiag1BbChunk[rank][file] = chunk;
		}
		cnt += diagLen;
	}
}


void initPntToDiag2BbChunkTable(){
	int chunk = 0;
	int cnt = 0;
	int diagStartRank;
	int diagStartFile;

	//count lower triangle chunks
	diagStartFile = 0;
	for (diagStartRank = 0; diagStartRank <= 19; diagStartRank++){;
		int diagLen = diagStartRank + 1;
		if (cnt + diagLen >= 64 ){
			// diag will cause overflow, must add diag to next chunk
			chunk++;
			cnt = 0;
		}
		//count along diagonal and add them to chunk
		for(int i = 0; i < diagLen; i++){
			int rank = diagStartRank - i;
			int file = diagStartFile + i;
			pntToDiag2BbChunk[rank][file] = chunk;
		}
		cnt += diagLen;
	}

	//count upper triange chunks
	diagStartRank = 19;
	for (diagStartFile = 1; diagStartFile <= 19; diagStartFile++){
		int diagLen = 20 - diagStartFile;	
		if (cnt + diagLen >= 64 ){
			// diag will cause overflow, must add diag to next chunk
			chunk++;
			cnt = 0;
		}
		//count along diagonal and add them to chunk
		for(int i = 0; i < diagLen; i++){
			int rank = diagStartRank - i;
			int file = diagStartFile + i;
			pntToDiag2BbChunk[rank][file] = chunk;
		}
		cnt += diagLen;
	}
}


void initGame(){
	g_numMoves = 0;
	memset(g_game, 0, sizeof(g_game));
	initPntToRankBbChunkTable();
	initPntToFileBbChunkTable();
	initPntToDiag1BbChunkTable();
	initPntToDiag2BbChunkTable();
}



/*=====================================================

				Printing functions

=======================================================*/

void printTable(int *arr, int rows, int cols){
	for(int i = rows - 1; i >= 0; i--){
		for(int k = 0; k < cols; k++){
			printf("%d ", *(arr+rows*i+k));
		}
		printf("\n");
	}
	printf("\n");
}

void printRankBb(Bitboard rankBb){
	for (int rank = 19; rank >= 0; rank--){
		printf("%2d ", rank);
		for(int file = 0; file <= 19; file++){	
			int chunk = pntToRankBbChunk[rank][file];
			int index = pntToRankBbIndex[rank][file];
			int bit = getBit(rankBb.bbChunks[chunk], index);
			printf("%d ", bit); 
		}
		printf("\n");
	}	
	printf("   ");
	for (int file = 0; file <=19; file++){
		printf("%c ", file + 'a');
	}
	printf("\n");
}


/*====================================================
					main stuff
====================================================*/

void printPrecalcs(){
	printf("pntToRankBbChunk\n");
	printTable((int *)pntToRankBbChunk, 20, 20);
	printf("pntToFileBbChunk\n");
	printTable((int *)pntToFileBbChunk, 20, 20);
	printf("pntToDiag1BbChunk\n");
	printTable((int *)pntToDiag1BbChunk, 20, 20);
	printf("pntToDiag2BbChunk\n");
	printTable((int *)pntToDiag2BbChunk, 20, 20);
}

int main(){
	initGame ();
	printPrecalcs();
}
