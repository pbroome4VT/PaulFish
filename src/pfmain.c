#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


#define GAME_MAX_MOVES (371)
#define BB_BITS (448)		//7*64

/*======================================
			Game types
========================================*/


typedef enum PlayerEnum Player_t;
enum PlayerEnum {BLACK = 0, WHITE = 1};

char playerToChar(Player_t player){
	return player == BLACK? 'B' : 'W';
}

typedef struct BitboardStruct Bitboard;
struct BitboardStruct{
	uint64_t bitChunk[7];
};



typedef struct GameStruct Game;
struct GameStruct{
	// state variables
	char captures [2];		/* all bitboard indexed by Player_t. example horzBB[BLACK] */
	Bitboard occupancies[2];
	Bitboard star1Occupancies;
};


int g_numMoves;
Game g_game[GAME_MAX_MOVES];


/*=====================================================

				Precalculated Tables/bitboards

=======================================================*/
int bitToChunkTable[BB_BITS];
int bitToChunkIndexTable[BB_BITS];
uint64_t chunkIndexToU64Table[64];
uint64_t invChunkIndexToU64Table[64];
Bitboard inBoundsMask;
Bitboard star1MaskTable[BB_BITS];
Bitboard star2MaskTable[BB_BITS];



/*=====================================================

				Bitboard Functions

=======================================================*/

Bitboard *bbAnd(Bitboard *bb1, Bitboard *bb2){
	bb1->bitChunk[0] &= bb2->bitChunk[0];
	bb1->bitChunk[1] &= bb2->bitChunk[1];
	bb1->bitChunk[2] &= bb2->bitChunk[2];
	bb1->bitChunk[3] &= bb2->bitChunk[3];
	bb1->bitChunk[4] &= bb2->bitChunk[4];
	bb1->bitChunk[5] &= bb2->bitChunk[5];
	bb1->bitChunk[6] &= bb2->bitChunk[6];
	return bb1;
}

Bitboard *bbOr(Bitboard *bb1, Bitboard *bb2){
	bb1->bitChunk[0] |= bb2->bitChunk[0];
	bb1->bitChunk[1] |= bb2->bitChunk[1];
	bb1->bitChunk[2] |= bb2->bitChunk[2];
	bb1->bitChunk[3] |= bb2->bitChunk[3];
	bb1->bitChunk[4] |= bb2->bitChunk[4];
	bb1->bitChunk[5] |= bb2->bitChunk[5];
	bb1->bitChunk[6] |= bb2->bitChunk[6];
	return bb1;
}

Bitboard *bbXor(Bitboard *bb1, Bitboard *bb2){
	bb1->bitChunk[0] ^= bb2->bitChunk[0];
	bb1->bitChunk[1] ^= bb2->bitChunk[1];
	bb1->bitChunk[2] ^= bb2->bitChunk[2];
	bb1->bitChunk[3] ^= bb2->bitChunk[3];
	bb1->bitChunk[4] ^= bb2->bitChunk[4];
	bb1->bitChunk[5] ^= bb2->bitChunk[5];
	bb1->bitChunk[6] ^= bb2->bitChunk[6];
	return bb1;
}

Bitboard *bbInv(Bitboard *bb){
	bb->bitChunk[0] = ~(bb->bitChunk[0]);
	bb->bitChunk[1] = ~(bb->bitChunk[1]);
	bb->bitChunk[2] = ~(bb->bitChunk[2]);
	bb->bitChunk[3] = ~(bb->bitChunk[3]);
	bb->bitChunk[4] = ~(bb->bitChunk[4]);
	bb->bitChunk[5] = ~(bb->bitChunk[5]);
	bb->bitChunk[6] = ~(bb->bitChunk[6]);
}

Bitboard *bbRsh(Bitboard *bb, int n){
	for (int i = 0; i < n; i++){	
		bb->bitChunk[0] = ( bb->bitChunk[1] & 1ULL ) ? ( bb->bitChunk[0] >> 1 | 0x8000ULL ) : ( bb->bitChunk[0] >> 1 );
		bb->bitChunk[1] = ( bb->bitChunk[2] & 1ULL ) ? ( bb->bitChunk[1] >> 1 | 0x8000ULL ) : ( bb->bitChunk[1] >> 1 );
		bb->bitChunk[2] = ( bb->bitChunk[3] & 1ULL ) ? ( bb->bitChunk[2] >> 1 | 0x8000ULL ) : ( bb->bitChunk[2] >> 1 );
		bb->bitChunk[3] = ( bb->bitChunk[4] & 1ULL ) ? ( bb->bitChunk[3] >> 1 | 0x8000ULL ) : ( bb->bitChunk[3] >> 1 );
		bb->bitChunk[4] = ( bb->bitChunk[5] & 1ULL ) ? ( bb->bitChunk[4] >> 1 | 0x8000ULL ) : ( bb->bitChunk[4] >> 1 );
		bb->bitChunk[5] = ( bb->bitChunk[6] & 1ULL ) ? ( bb->bitChunk[5] >> 1 | 0x8000ULL ) : ( bb->bitChunk[5] >> 1 );
		bb->bitChunk[6] = ( bb->bitChunk[6] >> 1 );
	}
	return bb;
}

/*int u64ToLS1BIndex(uint64_t x){
	int i = 0;
	x = ~x;
	while( x & 1ULL){
		x = x >> 1;
		i++;
	}
	return i;
}
*/
int u64ToLS1BIndex(uint64_t x){
	int i = 0;
	if((x & 0x00000000FFFFFFFFULL) == 0){
		i += 32;
		x >>= 32;
	}
	if((x & 0x000000000000FFFFULL) == 0){
		i += 16;
		x >>= 16;
	}
	if((x & 0x00000000000000FFULL) == 0){
		i += 8;
		x >>= 8;
	}
	if((x & 0x000000000000000FULL) == 0){
		i += 4;
		x >>= 4;
	}
	if((x & 0x0000000000000003ULL) == 0){
		i+= 2;
		x >>= 2;
	}
	if((x & 0x0000000000000001ULL) == 0){
		i+=1;
	}
	return i;
}

int bbGetLS1B(Bitboard *b){
	if(b->bitChunk[0]){ return u64ToLS1BIndex(b->bitChunk[0]); }
	if(b->bitChunk[1]){ return 64 + u64ToLS1BIndex(b->bitChunk[1]); }
	if(b->bitChunk[2]){ return 128 + u64ToLS1BIndex(b->bitChunk[2]); }
	if(b->bitChunk[3]){ return 192 + u64ToLS1BIndex(b->bitChunk[3]); }
	if(b->bitChunk[4]){ return 256 + u64ToLS1BIndex(b->bitChunk[4]); }
	if(b->bitChunk[5]){ return 320 + u64ToLS1BIndex(b->bitChunk[5]); }
	if(b->bitChunk[6]){ return 384 + u64ToLS1BIndex(b->bitChunk[6]); }
	return -1;
}

int countU64Bits(uint64_t x){
	int count = 0;
	while (x){
		x &= (x-1);
		count++;
	}
	return count;
}

int bbCountBits(Bitboard *b){
	int count = 0;
	if(b->bitChunk[0]){ count += countU64Bits(b->bitChunk[0]); }
	if(b->bitChunk[1]){ count += countU64Bits(b->bitChunk[1]); }
	if(b->bitChunk[2]){ count += countU64Bits(b->bitChunk[2]); }
	if(b->bitChunk[3]){ count += countU64Bits(b->bitChunk[3]); }
	if(b->bitChunk[4]){ count += countU64Bits(b->bitChunk[4]); }
	if(b->bitChunk[5]){ count += countU64Bits(b->bitChunk[5]); }
	if(b->bitChunk[6]){ count += countU64Bits(b->bitChunk[6]); }
	return count;
}

int bbNz(Bitboard *bb){
	return bb->bitChunk[0] || bb->bitChunk[1] || bb->bitChunk[2] || bb->bitChunk[3] || bb->bitChunk[4] || bb->bitChunk[5] || bb->bitChunk[6];
}

Bitboard *setBit(Bitboard *bb, int bit){
	bb->bitChunk[bitToChunkTable[bit]] |= chunkIndexToU64Table[bitToChunkIndexTable[bit]];
	return bb;
}

Bitboard *unsetBit(Bitboard *bb, int bit){
	bb->bitChunk[bitToChunkTable[bit]] &= invChunkIndexToU64Table[bitToChunkIndexTable[bit]];
	return bb;
}
int getBit(Bitboard *bb, int bit){
	return bb->bitChunk[bitToChunkTable[bit]] & (chunkIndexToU64Table[bitToChunkIndexTable[bit]])   ? 1:0;
}

int pntToBit(int rank, int file){
	return rank * 20 + file;
}

int bitToRank(int bit){
	return bit / 20;
}

int bitToFile(int bit){
	return bit % 20;
}
/*=====================================================

				init functions

=======================================================*/


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

void initChunkIndexToU64Tables(){
	for (int i = 0; i < 64; i++){
		chunkIndexToU64Table[i] = (1ULL << i);
		invChunkIndexToU64Table[i] = ~chunkIndexToU64Table[i];
	}
}


void initInBoundsMask(){
	memset(&inBoundsMask, 0, sizeof(Bitboard));
	for( int rank = 0; rank <= 19; rank++){
		for (int file = 0; file <= 19; file++){
			if(rank < 19 && file < 19){
				int bit = pntToBit(rank, file);
				setBit(&inBoundsMask, bit);
			}
		}
	}
}

void initStarMaskTables(){
	for (int rank = 0; rank <= 19; rank++){
		for (int file = 0; file <= 19; file++){
			int tableIndex = pntToBit(rank,file);
			if(getBit(&inBoundsMask, tableIndex) == 0){continue;} //boundary bits in table
			for(int rankDir = -1; rankDir <= 1; rankDir++){
				for(int fileDir = -1; fileDir <= 1; fileDir++){
					int bitOffset = pntToBit(rankDir, fileDir);
					int bit = tableIndex + bitOffset;
					if(bit >= 0 && bit < BB_BITS && getBit(&inBoundsMask, bit)){
						setBit(&star1MaskTable[tableIndex], bit);
						setBit(&star2MaskTable[tableIndex], bit);
						bit = tableIndex + 2*bitOffset;
						if (bit >= 0 && bit < BB_BITS && getBit(&inBoundsMask, bit)){
							setBit(&star2MaskTable[tableIndex], bit);
						}
					}
				}
			}
		}
	}
}


void initGame(){
	g_numMoves = 0;
	memset(g_game, 0, sizeof(g_game));
	initBitToChunkTable();
	initBitToChunkIndexTable();
	initChunkIndexToU64Tables();
	initInBoundsMask();
	initStarMaskTables();
}


/*=====================================================

				Helper functions

=======================================================*/

Player_t getOpp(Player_t player){
	return player ? BLACK : WHITE;
}

int playerGetOcc(int bit, Player_t player){
	return getBit(&(g_game[g_numMoves].occupancies[player]), bit);
}

void playerSetOcc(int bit, Player_t player){
	setBit(&(g_game[g_numMoves].occupancies[player]), bit);
}

void playerUnsetOcc(int bit, Player_t player){
	unsetBit(&(g_game[g_numMoves].occupancies[player]), bit);
}

int playerGetCaps(Player_t player){
	return g_game[g_numMoves].captures[player];
}
void playerAddCaps(int caps, Player_t player){
	g_game[g_numMoves].captures[player] += caps;
}

/*=====================================================

				Game functions

=======================================================*/
int makeCaptures(int bit, Player_t player){
	int numCaps = 0;
	Player_t opp = getOpp(player);
	//E
	if( playerGetOcc(bit+1, opp) &&
		playerGetOcc(bit+2, opp) &&
		playerGetOcc(bit+3, player))
	{
		numCaps++;
		playerUnsetOcc(bit+1, opp);
		playerUnsetOcc(bit+2, opp);
	}
	//NE 
	if( playerGetOcc(bit+21, opp) &&
		playerGetOcc(bit+42, opp) &&
		playerGetOcc(bit+63, player))
	{
		numCaps++;
		playerUnsetOcc(bit+21, opp);
		playerUnsetOcc(bit+42, opp);
	}
	//N
	if( playerGetOcc(bit+20, opp) &&
		playerGetOcc(bit+40, opp) &&
		playerGetOcc(bit+60, player))
	{
		numCaps++;
		playerUnsetOcc(bit+20, opp);
		playerUnsetOcc(bit+40, opp);
	}
	//NW
	if( playerGetOcc(bit+19, opp) &&
		playerGetOcc(bit+38, opp) &&
		playerGetOcc(bit+57, player))
	{
		numCaps++;
		playerUnsetOcc(bit+19, opp);
		playerUnsetOcc(bit+38, opp);
	}
	//W
	if( bit - 3 >= 0 &&
		playerGetOcc(bit-1, opp) &&
		playerGetOcc(bit-2, opp) &&
		playerGetOcc(bit-3, player))
	{
		numCaps++;
		playerUnsetOcc(bit-1, opp);
		playerUnsetOcc(bit-2, opp);
	}
	// SW
	if( bit-63 >= 0 &&
		playerGetOcc(bit-21, opp) &&
		playerGetOcc(bit-42, opp) &&
		playerGetOcc(bit-63, player))
	{
		numCaps++;
		playerUnsetOcc(bit-21, opp);
		playerUnsetOcc(bit-42, opp);
	}
	// S
	if( bit - 60 >= 0 &&
		playerGetOcc(bit-20, opp) &&
		playerGetOcc(bit-40, opp) &&
		playerGetOcc(bit-60, player))
	{
		numCaps++;
		playerUnsetOcc(bit-20, opp);
		playerUnsetOcc(bit-40, opp);
	}
	//SE
	if( bit - 57 > 0 &&
		playerGetOcc(bit-19, opp) &&
		playerGetOcc(bit-38, opp) &&
		playerGetOcc(bit-57, player))
	{
		numCaps++;
		playerUnsetOcc(bit-19, opp);
		playerUnsetOcc(bit-38, opp);
	}
	return numCaps;
}

int isConnect5(int bitIndex, Player_t player){
	int streak;
	
	//check rank
	streak = 1;
	for(int i = 1; i < 5; i++){
		int offset = 1;
		int bit = bitIndex + i * offset;
		if(playerGetOcc(bit, player)){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -1;
		int bit = bitIndex + i * offset;
		if(bit >= 0 && playerGetOcc(bit, player)){
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
		if(playerGetOcc(bit, player)){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -21;
		int bit = bitIndex + i * offset;
		if(bit >= 0 && playerGetOcc(bit, player)){
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
		if(playerGetOcc(bit, player)){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -20;
		int bit = bitIndex + i * offset;
		if(bit >= 0 && playerGetOcc(bit, player)){
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
		if(playerGetOcc(bit, player)){
			streak++;
		}else{
			break;
		}
	}
	for(int i = 1; i < 5; i++){
		int offset = -19;
		int bit = bitIndex + i * offset;
		if(bit >= 0 && playerGetOcc(bit, player)){
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


void play(int bit, Player_t player){
	g_numMoves++;
	memcpy( g_game + g_numMoves, g_game + g_numMoves - 1, sizeof(*g_game));
	playerSetOcc(bit, player);
	playerAddCaps(makeCaptures(bit, player), player);
	bbOr( &(g_game[g_numMoves].star1Occupancies), &(star1MaskTable[bit]));
}

void undo(){
	g_numMoves--;
}


/*=====================================================

				  AI data+functions

=======================================================*/

void playFast(int bit, Player_t player){
	play(bit, player);
}

Bitboard getMoves(){
	Bitboard b;
	b = g_game[g_numMoves].occupancies[BLACK];
	bbOr(&b, &(g_game[g_numMoves].occupancies[WHITE]));
	bbAnd(bbInv(&b), &inBoundsMask);
	return b;
}

Bitboard getMovesStar1(){
	Bitboard b = g_game[g_numMoves].star1Occupancies;
	bbXor(&b, &(g_game[g_numMoves].occupancies[BLACK]));
	bbXor(&b, &(g_game[g_numMoves].occupancies[WHITE]));
	return b;
}

#define MAXIMIZER BLACK
#define MINIMIZER WHITE

typedef struct FrameStatsStruct FrameStats;
struct FrameStatsStruct{
	int singles;
	int doubles;
	int triples;
	int quadruples;
};


FrameStats getFrameStats(){
	Game frame = g_game[g_numMoves];
	FrameStats stats;
	//count singles
	int singles;
	singles = bbCountBits(&(frame.occupancies[MAXIMIZER])) - bbCountBits(&(frame.occupancies[MINIMIZER]));

	stats.singles = singles;
	return stats;
};

const int SINGLES_WEIGHT = 1;
const int DOUBLES_WEIGHT = 2;
const int TRIPLES_WEIGHT = 3;
const int QUADRUPLES_WEIGHT = 4;
int heuristic(Game *frame){
	return 0;		
}

typedef struct EvalStruct Eval;
struct EvalStruct{
	int score;
	int move;
};


long int g_nodes;	
Eval minimax(Player_t player, int depth, int alpha, int beta){
	Eval e;
	if(depth){
		Bitboard moves = getMovesStar1();
		int move;
		if(player == MAXIMIZER){
			// maximizer
			e.score = -1001;
			e.move = -1;
			while ( (move = bbGetLS1B(&moves)) != -1 ){
				unsetBit(&moves, move);
				playFast(move, MAXIMIZER);
				if(isConnect5(move, MAXIMIZER)){
					e.score = 1000;
					e.move = move;
					undo();
					return e;
				}
				Eval tmp = minimax(MINIMIZER, depth - 1, alpha, beta);
				if(tmp.score > e.score){
					e.score = tmp.score;
					e.move = move;
					if(e.score > beta){
						undo();
						return e;
					}
				}
				if(alpha < e.score){
					alpha = e.score;
				}
				undo();
			}
			return e;
		}else{
			//minimizer	
			e.score = 1001;
			e.move = -1;
			while ( (move = bbGetLS1B(&moves)) != -1 ){
				unsetBit(&moves, move);
				playFast(move, MINIMIZER);
				if(isConnect5(move, MINIMIZER)){
					e.score = -1000;
					e.move = move;
					undo();
					return e;
				}
				Eval tmp = minimax(MAXIMIZER, depth - 1, alpha, beta);
				if(tmp.score < e.score){
					e.score = tmp.score;
					e.move = move;
					if(e.score < alpha){
						undo();
						return e;
					}
					if(beta > e.score){
						beta = e.score;
					}
				}
				undo();
			}
			return e;
		}
	}
	g_nodes++;
	e.score = 0;
	e.move = -1;
	return e;
}


Eval g_paulFishEval;
typedef struct PaulFishArgsStruct PaulFishArgs;
struct PaulFishArgsStruct{
	Player_t player;
	int maxDepth;
};



void *computeJob(void *arguments){
	int retStatus;
	PaulFishArgs *args = (PaulFishArgs *)arguments;
	Player_t player = args->player;
	int maxDepth = args->maxDepth;
	retStatus = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	if(retStatus != 0){
		perror("ERROR:computeJob(), pthread_setcanceltype() failed\n");
		exit(1);
	}
	for(int depth = 1; depth <= maxDepth; depth++){	
		clock_t t = clock();
		g_paulFishEval = minimax(player, depth, -10000 , 10000);
		t = clock() - t;
		double timeTaken = (double)t / CLOCKS_PER_SEC;
		printf("depth:%d     moveRec:%d     nodes:%ld     time%lf     nodes/sec:%lf\n",depth, g_paulFishEval.move, g_nodes, timeTaken, g_nodes/timeTaken);
	}
}

Eval paulFish(PaulFishArgs args, int seconds){
	int retStatus; 
	pthread_t thread;
	void *res;
	
	g_nodes = 0;
	int currentMove = g_numMoves; //need this to restore board state when minimax gets interrupted halfway through calc
	
	retStatus = pthread_create(&thread, NULL, &computeJob, &args);
	if(retStatus != 0){
		perror("ERROR: Paulfish(), pthread_create()\n");
		exit(1);
	}
	sleep(seconds);
	pthread_cancel(thread);
	if(retStatus != 0){
		perror ("ERROR: Paulfish(), pthread_cancel()\n");
		exit(1);
	}
	retStatus = pthread_join(thread, &res);
	if(retStatus != 0 ){
		perror("ERROR: Paulfish(), pthread_join()\n");
		exit(1);
	}
	g_numMoves = currentMove;
	return g_paulFishEval;
}


/*=====================================================

				Printing functions

=======================================================*/


void printBbString(Bitboard b){
	for(int i = BB_BITS-1; i >= 0; i--){	
		printf("%d", getBit(&b, i)? 1 : 0);
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
	printf("Black Captures %d\t WhiteCaptures %d\n", playerGetCaps(BLACK), playerGetCaps(WHITE));
}

void printBb(Bitboard bb){
	for(int rank = 19; rank >= 0; rank--){
		printf("%2d  ", rank);
		for(int file = 0; file <= 19; file++){
			int bit = pntToBit(rank,file);
			char c;
			if(getBit(&bb, bit)){
				c = '1';
			}else{
				c = '0';
			}
			printf("%c ", c);
		}
		printf("\n");
	}
	printf("    ");
	for(int file = 0; file <= 19; file++){
		printf("%c ", 'a' + file);
	}
	printf("\n");
}

void printFrameStats(FrameStats stats){
	printf("FrameStats:\n");
	printf("\tsingles : %d\n\tdoubles : %d\n\ttriples : %d\n\tquadruples : %d\n", stats.singles, stats.doubles, stats.triples, stats.quadruples);
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
	play(pntToBit(8,8), WHITE);
	play(pntToBit(9,9), WHITE);
	play(pntToBit(7,9), WHITE);
	play(pntToBit(8,9), BLACK);
	play(pntToBit(9,8), BLACK);
	play(pntToBit(7,8), BLACK);
}


int main(){	
	initGame();
	//testGame();
	/*
	PaulFishArgs args;
	args.player= WHITE;
	args.maxDepth = 8;
	Eval e = paulFish(args, 45);
	printf("score %d\tmove%d\n", e.score, e.move);
	play(e.move, WHITE);
	*/

	Player_t player = BLACK;
	int rankIn;
	char fileIn;
	while(1){	
		printGame();
		printFrameStats(getFrameStats());
		printf("player %c enter move: ", playerToChar(player));
		int n =scanf("%c%d", &fileIn, &rankIn);
		while (getchar() != '\n'){};
		int rank = rankIn;
		int file = fileIn - 'a';
		int bit = pntToBit(rank,file);
		play(bit, player);
		if(isConnect5(bit, player)){
			printf("GAME OVER\n");
			break;
		}
		player = getOpp(player);
		printf("black #occs = %d\n", bbCountBits(&(g_game[g_numMoves].occupancies[BLACK])));
		/*printGame();
		PaulFishArgs args = {WHITE, 10};
		Eval e = paulFish(args, 5);
		printf("score %d\tmove(%c,%d)\n", e.score, bitToFile(e.move)+'a', bitToRank(e.move));
		play(e.move, WHITE);
		if(isConnect5(bit, BLACK)){
			printf("GAME OVER\n");
			break;
		}*/
	}
	printGame();
	
	/*Player_t player = BLACK;
	int rank, file;
	while(1){
		printBb(g_game[g_numMoves].occupancies[BLACK]);
		printBb(g_game[g_numMoves].occupancies[WHITE]);
		printBb(getMovesStar1());
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
	*/
}
