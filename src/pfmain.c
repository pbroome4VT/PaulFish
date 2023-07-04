#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#define GAME_MAX_MOVES (371)
#define BB_BITS (448)		//7*64

/*======================================
			Game types
========================================*/


typedef enum PlayerEnum Player_t;
enum PlayerEnum {BLACK = 0, WHITE = 1};



typedef struct BitboardStruct Bitboard;
struct BitboardStruct{
	uint64_t bitChunk[7];
};



typedef struct GameStruct Game;
struct GameStruct{
	// state variables
	char captures [2];		/* all bitboard indexed by Player_t. example horzBB[BLACK] */
	Bitboard occupancies[2];

};


int g_numMoves;
Game g_game[GAME_MAX_MOVES];


/*=====================================================

				Precalculated Tables

=======================================================*/
Bitboard bitIndexSetTable[BB_BITS];
Bitboard bitIndexUnsetTable[BB_BITS];
int bitToChunkTable[BB_BITS];
int bitToChunkIndexTable[BB_BITS];


/*=====================================================

				init functions

=======================================================*/
void initBitIndexSetTable(){
	for(int i = 0; i < BB_BITS; i++){
		int chunk = i/64;
		int index = i%64;
		bitIndexSetTable[i].bitChunk[chunk] = 1ULL << index;
	}
}

void initBitIndexUnsetTable(){
	for( int i = 0; i < BB_BITS; i++){
		int chunk = i/64;
		int index = i%64;
		Bitboard b;
		memset(&b, 0xFF, sizeof(b));
		b.bitChunk[chunk] ^= (1ULL << index);
		bitIndexUnsetTable[i] = b;
	}
}

void initBitToChunkTable(){
	for(int i = 0; i < BB_BITS; i++){
		bitToChunkTable[i] = i/64;
	}
}

void initBitToChunkIndexTable(){
	for(int i = 0; i < BB_BITS; i++){
		bitToChunkIndexTable[i] = i%64;
	}
}

void initGame(){
	g_numMoves = 0;
	memset(g_game, 0, sizeof(g_game));
	initBitIndexSetTable();
	initBitIndexUnsetTable();
	initBitToChunkTable();
	initBitToChunkIndexTable();
}



/*=====================================================

				Bitboard Functions

=======================================================*/

Bitboard bbAnd(Bitboard bb1, Bitboard bb2){
	bb1.bitChunk[0] &= bb2.bitChunk[0];
	bb1.bitChunk[1] &= bb2.bitChunk[1];
	bb1.bitChunk[2] &= bb2.bitChunk[2];
	bb1.bitChunk[3] &= bb2.bitChunk[3];
	bb1.bitChunk[4] &= bb2.bitChunk[4];
	bb1.bitChunk[5] &= bb2.bitChunk[5];
	bb1.bitChunk[6] &= bb2.bitChunk[6];
	return bb1;
}

Bitboard bbOr(Bitboard bb1, Bitboard bb2){
	bb1.bitChunk[0] |= bb2.bitChunk[0];
	bb1.bitChunk[1] |= bb2.bitChunk[1];
	bb1.bitChunk[2] |= bb2.bitChunk[2];
	bb1.bitChunk[3] |= bb2.bitChunk[3];
	bb1.bitChunk[4] |= bb2.bitChunk[4];
	bb1.bitChunk[5] |= bb2.bitChunk[5];
	bb1.bitChunk[6] |= bb2.bitChunk[6];
	return bb1;
}

Bitboard bbXor(Bitboard bb1, Bitboard bb2){
	bb1.bitChunk[0] ^= bb2.bitChunk[0];
	bb1.bitChunk[1] ^= bb2.bitChunk[1];
	bb1.bitChunk[2] ^= bb2.bitChunk[2];
	bb1.bitChunk[3] ^= bb2.bitChunk[3];
	bb1.bitChunk[4] ^= bb2.bitChunk[4];
	bb1.bitChunk[5] ^= bb2.bitChunk[5];
	bb1.bitChunk[6] ^= bb2.bitChunk[6];
	return bb1;
}

Bitboard bbRsh(Bitboard bb, int n){
	for (int i = 0; i < n; i++){	
		bb.bitChunk[0] = ( bb.bitChunk[1] & 1ULL ) ? ( bb.bitChunk[0] >> 1 | 0x8000ULL ) : ( bb.bitChunk[0] >> 1 );
		bb.bitChunk[1] = ( bb.bitChunk[2] & 1ULL ) ? ( bb.bitChunk[1] >> 1 | 0x8000ULL ) : ( bb.bitChunk[1] >> 1 );
		bb.bitChunk[2] = ( bb.bitChunk[3] & 1ULL ) ? ( bb.bitChunk[2] >> 1 | 0x8000ULL ) : ( bb.bitChunk[2] >> 1 );
		bb.bitChunk[3] = ( bb.bitChunk[4] & 1ULL ) ? ( bb.bitChunk[3] >> 1 | 0x8000ULL ) : ( bb.bitChunk[3] >> 1 );
		bb.bitChunk[4] = ( bb.bitChunk[5] & 1ULL ) ? ( bb.bitChunk[4] >> 1 | 0x8000ULL ) : ( bb.bitChunk[4] >> 1 );
		bb.bitChunk[5] = ( bb.bitChunk[6] & 1ULL ) ? ( bb.bitChunk[5] >> 1 | 0x8000ULL ) : ( bb.bitChunk[5] >> 1 );
		bb.bitChunk[6] = ( bb.bitChunk[6] >> 1 );
	}
	return bb;
}

int bbNz(Bitboard bb){
	return bb.bitChunk[0] || bb.bitChunk[1] || bb.bitChunk[2] || bb.bitChunk[3] || bb.bitChunk[4] || bb.bitChunk[5] || bb.bitChunk[6];
}

Bitboard setBit(Bitboard bb, int bit){
	bb.bitChunk[bitToChunkTable[bit]] |= (1ULL << bitToChunkIndexTable[bit]);
	return bb;
}
i
Bitboard unsetBit(Bitboard *bb, int bit){
	bb.bitChunk[bitToChunkTable[bit]] &= ~(1ULL << bitToChunkIndexTable[bit]);
	returb bb;
}
int getBit(Bitboard bb, int bit){
	return bb.bitChunk[bitToChunkTable[bit]] & (1ULL << bitToChunkIndexTable[bit])   ? 1:0;
}

/*=====================================================

				Helper functions

=======================================================*/
int pntToBit(int rank, int file){
	return rank * 20 + file;
}

Player_t getOpp(Player_t player){
	return player ? BLACK : WHITE;
}

int playerGetOcc(int bit, Player_t player){
	return getBit(g_game[g_numMoves].occupancies[player], bit);
}

void playerSetOcc(int bit, Player_t player){
	setBit(g_game[g_numMoves].occupancies[player], bit);
}

void playerUnsetOcc(int bit, Player_t player){
	unsetBit(g_game[g_numMoves].occupancies[player], bit);
}

/*=====================================================

				Game functions

=======================================================*/
int makeCaptures(int bitIndex, Player_t player){
	Player_t opp = getOpp(player);
	//check right
	if( playerGetOcc(bitIndex+1, opp) &&
		playerGetOcc(bitIndex+2, opp) &&
		playerGetOcc(bitIndex+3, player))
	{
		printf("capture\n");	
	}
}

int isConnect5(int bitIndex, Player_t player){
	int streak;
	
	//check rank
	streak = 1;
	for(int i = 1; i < 5; i++){
		int offset = 1;
		int bit = bitIndex + i * offset;
		if(getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -1;
		int bit = bitIndex + i * offset;
		if(bitIndex >= 0 && getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5){
		return 1;
	}
	//check diag1
	streak = 1;
	for(int i = 1; i < 5; i++){
		int offset = 21;
		int bit = bitIndex + i * offset;
		if(getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -21;
		int bit = bitIndex + i * offset;
		if(bitIndex >= 0 && getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5){
		return 1;
	}
	//check file
	streak = 1;
	for(int i = 1; i < 5; i++){
		int offset = 20;
		int bit = bitIndex + i * offset;
		if(getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -20;
		int bit = bitIndex + i * offset;
		if(bitIndex >= 0 && getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5){
		return 1;
	}
	//check diag2
	streak = 1;
	for(int i = 1; i < 5; i++){
		int offset = 19;
		int bit = bitIndex + i * offset;
		if(getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -19;
		int bit = bitIndex + i * offset;
		if(bitIndex >= 0 && getBit( g_game[g_numMoves].occupancies[player], bit )){
			streak++;
		}else{
			break;
		}
	}
	if(streak >= 5){
		return 1;
	}
	return 0;
}


void play(int bitIndex, Player_t player){
	g_numMoves++;
	memcpy( g_game + g_numMoves, g_game + g_numMoves - 1, sizeof(*g_game));
	setBit( g_game[g_numMoves].occupancies[player], bitIndex );
	//makeCaptures(bitIndex, player);
}

/*=====================================================

				Printing functions

=======================================================*/


void printBbString(Bitboard b){
	for(int i = BB_BITS-1; i >= 0; i--){	
		printf("%d", getBit(b, i));
		if (i%8 == 0){
			printf(" ");
		}
	}
	printf("\n");
}

void printGame(){
	for(int rank = 18; rank >= 0; rank--){
		printf("%2d  ", rank);
		for(int file = 0; file <= 18; file++){
			int bit = pntToBit(rank,file);
			char c;
			if(playerGetOcc(bit, BLACK)){
				c = 'B';
			}else if (playerGetOcc(bit, WHITE)){
				c = 'W';
			}else{
				c = '*';
			}
			printf("%c ", c);
		}
		printf("\n");
	}
	printf("    ");
	for(int file = 0; file <= 18; file++){
		printf("%c ", 'a' + file);
	}
	printf("\n");
}

void printTable(int *arr, int rows, int cols){
	for(int i = rows - 1; i >= 0; i--){
		for(int k = 0; k < cols; k++){
			printf("%2d ", *(arr+rows*i+k));
		}
		printf("\n");
	}
	printf("\n");
}

/*====================================================
					main stuff
====================================================*/
int testGame(){
	play(1, WHITE);
	play(2, WHITE);
	play(3, BLACK);
}


int main(){	
	initGame();
	testGame();
	Player_t player = BLACK;
	int rank, file;
	while(1){
		printBbString(g_game[g_numMoves].occupancies[WHITE]);
		printGame();
		scanf("%d%d", &file, &rank);
		while (getchar() != '\n'){};
		int bit = pntToBit(rank,file);
		play(bit, player);
		if(isConnect5(bit, player)){
			printf("GAME OVER\n");
			break;
		}
		player = getOpp(player);
	}
	printGame();

}
