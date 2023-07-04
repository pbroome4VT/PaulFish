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
	for (int i = 0; i < BB_BITS; i++){
	
	}
}


void initGame(){
	g_numMoves = 0;
	memset(g_game, 0, sizeof(g_game));
	initBitIndexSetTable();
}



/*=====================================================

				Bitboard Functions

=======================================================*/

Bitboard bbAnd(Bitboard bb1, Bitboard bb2){
	Bitboard b;
	b.bitChunk[0] = bb1.bitChunk[0] & bb2.bitChunk[0];
	b.bitChunk[1] = bb1.bitChunk[1] & bb2.bitChunk[1];
	b.bitChunk[2] = bb1.bitChunk[2] & bb2.bitChunk[2];
	b.bitChunk[3] = bb1.bitChunk[3] & bb2.bitChunk[3];
	b.bitChunk[4] = bb1.bitChunk[4] & bb2.bitChunk[4];
	b.bitChunk[5] = bb1.bitChunk[5] & bb2.bitChunk[5];
	b.bitChunk[6] = bb1.bitChunk[6] & bb2.bitChunk[6];
	return b;
}

Bitboard bbOr(Bitboard bb1, Bitboard bb2){
	Bitboard b;
	b.bitChunk[0] = bb1.bitChunk[0] | bb2.bitChunk[0];
	b.bitChunk[1] = bb1.bitChunk[1] | bb2.bitChunk[1];
	b.bitChunk[2] = bb1.bitChunk[2] | bb2.bitChunk[2];
	b.bitChunk[3] = bb1.bitChunk[3] | bb2.bitChunk[3];
	b.bitChunk[4] = bb1.bitChunk[4] | bb2.bitChunk[4];
	b.bitChunk[5] = bb1.bitChunk[5] | bb2.bitChunk[5];
	b.bitChunk[6] = bb1.bitChunk[6] | bb2.bitChunk[6];
	return b;
}

Bitboard bbXor(Bitboard bb1, Bitboard bb2){
	Bitboard b;
	b.bitChunk[0] = bb1.bitChunk[0] ^ bb2.bitChunk[0];
	b.bitChunk[1] = bb1.bitChunk[1] ^ bb2.bitChunk[1];
	b.bitChunk[2] = bb1.bitChunk[2] ^ bb2.bitChunk[2];
	b.bitChunk[3] = bb1.bitChunk[3] ^ bb2.bitChunk[3];
	b.bitChunk[4] = bb1.bitChunk[4] ^ bb2.bitChunk[4];
	b.bitChunk[5] = bb1.bitChunk[5] ^ bb2.bitChunk[5];
	b.bitChunk[6] = bb1.bitChunk[6] ^ bb2.bitChunk[6];
	return b;
}

Bitboard bbRsh(Bitboard bb, int n){
	Bitboard b = bb;
	for (int i = 0; i < n; i++){	
		b.bitChunk[0] = ( b.bitChunk[1] & 1ULL ) ? ( b.bitChunk[0] >> 1 | 0x8000ULL ) : ( b.bitChunk[0] >> 1 );
		b.bitChunk[1] = ( b.bitChunk[2] & 1ULL ) ? ( b.bitChunk[1] >> 1 | 0x8000ULL ) : ( b.bitChunk[1] >> 1 );
		b.bitChunk[2] = ( b.bitChunk[3] & 1ULL ) ? ( b.bitChunk[2] >> 1 | 0x8000ULL ) : ( b.bitChunk[2] >> 1 );
		b.bitChunk[3] = ( b.bitChunk[4] & 1ULL ) ? ( b.bitChunk[3] >> 1 | 0x8000ULL ) : ( b.bitChunk[3] >> 1 );
		b.bitChunk[4] = ( b.bitChunk[5] & 1ULL ) ? ( b.bitChunk[4] >> 1 | 0x8000ULL ) : ( b.bitChunk[4] >> 1 );
		b.bitChunk[5] = ( b.bitChunk[6] & 1ULL ) ? ( b.bitChunk[5] >> 1 | 0x8000ULL ) : ( b.bitChunk[5] >> 1 );
		b.bitChunk[6] = ( b.bitChunk[6] >> 1 );
	}
	return b;
}

int bbNz(Bitboard bb){
	return bb.bitChunk[0] || bb.bitChunk[1] || bb.bitChunk[2] || bb.bitChunk[3] || bb.bitChunk[4] || bb.bitChunk[5] || bb.bitChunk[6];
}


#define setBit(bb, index)		((bb) = bbOr((bb), bitIndexSetTable[(index)]))
#define unsetBit(bb, index)		((bb) = bbAnd((bb), bitIndexUnsetTable[(index)]))
#define getBit(bb, index) 		(bbNz(bbAnd((bb), bitIndexSetTable[(index)])))

/*=====================================================

				Helper functions

=======================================================*/
#define pntToBit(rank,file)    		((rank) * 20 + (file))
#define getOpp(player)  			(player ? BLACK : WHITE)
#define playerGetOcc(bit, player)	(getBit(g_game[g_numMoves].occupancies[(player)], (bit)))
#define playerSetOcc(bit, player)	(setBit(g_game[g_numMoves].occupancies[(player)], (bit)))

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
}

/*=====================================================

				Printing functions

=======================================================*/


void printBbString(Bitboard b){
	for(int i = BB_BITS-1; i > 0; i--){
		if (i%8 == 0){
			printf(" ");
		}
		printf("%d", getBit(b, i));
	}
}

void printGame(){
	for(int rank = 18; rank >= 0; rank--){
		printf("%2d  ", rank);
		for(int file = 0; file <= 18; file++){
			int bitIndex = pntToBit(rank,file);
			char c;
			if(getBit(g_game[g_numMoves].occupancies[BLACK], bitIndex)){
				c = 'B';
			}else if (getBit(g_game[g_numMoves].occupancies[WHITE], bitIndex)){
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
	play(0, BLACK);
	play(20, BLACK);
	play(40,BLACK);
	play(60, BLACK);
}


int main(){	
	initGame();
	testGame();
	Player_t player = BLACK;
	int rank, file;
	while(1){
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
