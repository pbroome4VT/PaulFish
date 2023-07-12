#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


#define GAME_BOARD_SQUARES (361)
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


typedef struct MoveStruct Move;
struct MoveStruct{
	int square;
	int score;
};

typedef struct GameStruct Game;
struct GameStruct{
	// state variables
	char captures [2];		/* all bitboard indexed by Player_t. example horzBB[BLACK] */
	Bitboard occupancies[2];
	Bitboard star1Occupancies;
	int candidateMovesLen;
	Move newCandidateMoves[GAME_BOARD_SQUARES];

	int candidateMoves[GAME_BOARD_SQUARES];
};


int g_numMoves;
Game g_game[GAME_MAX_MOVES];
int g_moves[GAME_MAX_MOVES];

/*=====================================================

				Precalculated Tables/bitboards

=======================================================*/

typedef enum Offset Offset;
enum Offset {EAST = 1, NORTH_EAST = 21, NORTH = 20, NORTH_WEST = 19, WEST = -1, SOUTH_WEST = -21, SOUTH = -20, SOUTH_EAST = -19, NUM_OFFSETS = 8};
const Offset PRINCIPAL_DIRECTION[] = {EAST, NORTH_EAST, NORTH, NORTH_WEST};		// list of the positive offset directions
const Offset DIRECTION[] = {EAST, NORTH_EAST, NORTH, NORTH_WEST, WEST, SOUTH_WEST, SOUTH, SOUTH_EAST};

int bitToChunkTable[BB_BITS];
int bitToChunkIndexTable[BB_BITS];
uint64_t chunkIndexToU64Table[64];
uint64_t invChunkIndexToU64Table[64];
Bitboard inBoundsMask;
Bitboard star1MaskTable[BB_BITS];
Bitboard star2MaskTable[BB_BITS];
int bitDistanceFromEdgeTable[BB_BITS];



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
	return bb;
}

//(p & ~ q)
Bitboard *bbPAndNotQ(Bitboard *p, Bitboard *q){
	p->bitChunk[0] &= ~q->bitChunk[0];	
	p->bitChunk[1] &= ~q->bitChunk[1];	
	p->bitChunk[2] &= ~q->bitChunk[2];	
	p->bitChunk[3] &= ~q->bitChunk[3];	
	p->bitChunk[4] &= ~q->bitChunk[4];	
	p->bitChunk[5] &= ~q->bitChunk[5];	
	p->bitChunk[6] &= ~q->bitChunk[6];	
}

//ONLY CALL WHEN SHIFTING LESS THAN 64 BITS AT A TIME!!
Bitboard *bbLsh(Bitboard *bb, int n){
	bb->bitChunk[6] = bb->bitChunk[6] << n | (bb->bitChunk[5] >> (64-n));
	bb->bitChunk[5] = bb->bitChunk[5] << n | (bb->bitChunk[4] >> (64-n));
	bb->bitChunk[4] = bb->bitChunk[4] << n | (bb->bitChunk[3] >> (64-n));
	bb->bitChunk[3] = bb->bitChunk[3] << n | (bb->bitChunk[2] >> (64-n));
	bb->bitChunk[2] = bb->bitChunk[2] << n | (bb->bitChunk[1] >> (64-n));
	bb->bitChunk[1] = bb->bitChunk[1] << n | (bb->bitChunk[0] >> (64-n));
	bb->bitChunk[0] = bb->bitChunk[0] << n; 
	return bb;
}

//ONLY CALL WHEN SHIFTING LESS THAN 64 BITS AT A TIME!!
Bitboard *bbRsh(Bitboard *bb, int n){
	bb->bitChunk[0] = bb->bitChunk[0] >> n | (bb->bitChunk[1] << (64-n));
	bb->bitChunk[1] = bb->bitChunk[1] >> n | (bb->bitChunk[2] << (64-n));
	bb->bitChunk[2] = bb->bitChunk[2] >> n | (bb->bitChunk[3] << (64-n));
	bb->bitChunk[3] = bb->bitChunk[3] >> n | (bb->bitChunk[4] << (64-n));
	bb->bitChunk[4] = bb->bitChunk[4] >> n | (bb->bitChunk[5] << (64-n));
	bb->bitChunk[5] = bb->bitChunk[5] >> n | (bb->bitChunk[6] << (64-n));
	bb->bitChunk[6] = bb->bitChunk[6] >> n; 
	return bb;
}

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

//counts # of occurances of exactly n sequential bits
int bbCountOffsetStreakOfN(Bitboard *bb, int offset, int n){
	Bitboard cpy = *bb;	
	Bitboard tmp = *bb;
	bbInv(&cpy);
	for(int shift = 1; shift <= n; shift++){
		bbLsh(&tmp, offset);
		bbAnd(&cpy, &tmp);
	}
	bbLsh(&tmp, offset);
	bbInv(&tmp);
	bbAnd(&cpy, &tmp);
	return bbCountBits(&cpy);
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
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

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

void initBitDistanceFromEdgeTable(){
	for(int bit = 0 ; bit < BB_BITS; bit++){
		int rank = bitToRank(bit);
		int file = bitToFile(bit);
		int rankDist = MIN(rank, 18-rank);
		int fileDist = MIN(file, 18- file);
		int dist = MIN(rankDist, fileDist);
		bitDistanceFromEdgeTable[bit] = dist;
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
	initBitDistanceFromEdgeTable();
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


void appendCandidateMove(int bit){
	Game *frame = &(g_game[g_numMoves]);
	frame->newCandidateMoves[frame->candidateMovesLen++].square = bit;
}

Move getCandidateMove(int index){
	return g_game[g_numMoves].newCandidateMoves[index];
}

void setCandidateMove(int index, Move move){
	 g_game[g_numMoves].newCandidateMoves[index] = move;
}

int getCandidateMoveSquare(int index){
	return g_game[g_numMoves].newCandidateMoves[index].square;
}

int getCandidateMoveScore(int index){
	return g_game[g_numMoves].newCandidateMoves[index].score;	
}

void setCandidateMoveScore(int index, int score){
	g_game[g_numMoves].newCandidateMoves[index].score = score;
}

void swapCandidateMoves(int i, int k){
	Move tmp = getCandidateMove(i);
	setCandidateMove(i, getCandidateMove(k));
	setCandidateMove(k, tmp);
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

void printBbString(Bitboard b){
	for(int i = BB_BITS-1; i >= 0; i--){	
		printf("%d", getBit(&b, i)? 1 : 0);
		if (i%8 == 0){
			printf(" ");
		}
	}
	printf("\n\n");
}

/*=====================================================

				Game functions

=======================================================*/


int makeCaptures(int bit, Player_t player){
	int numCaps = 0;
	Player_t opp = getOpp(player);
	for(int direction = 0; direction < 4; direction++){
		Offset offset = PRINCIPAL_DIRECTION[direction];
		if(
			playerGetOcc(bit + offset, opp) &&
			playerGetOcc(bit + 2 * offset, opp) &&
			playerGetOcc(bit + 3 * offset, player))
		{
			numCaps++;
			playerUnsetOcc(bit + offset, opp);
			playerUnsetOcc(bit + 2 * offset, opp);
		}
		if(
			bit - 3 * offset >= 0 &&
			playerGetOcc(bit - offset, opp) &&
			playerGetOcc(bit - 2 * offset, opp) &&
			playerGetOcc(bit - 3 * offset, player))
		{
			numCaps++;
			playerUnsetOcc(bit - offset, opp);
			playerUnsetOcc(bit - 2 * offset, opp);
		}
	}
	return numCaps;
}


int isCapture(int bit, Player_t player){
	int numCaps = 0;
	Player_t opp = getOpp(player);
	for(int direction = 0; direction < 4; direction++){
		Offset offset = PRINCIPAL_DIRECTION[direction];
		if(
			playerGetOcc(bit + offset, opp) &&
			playerGetOcc(bit + 2 * offset, opp) &&
			playerGetOcc(bit + 3 * offset, player))
		{
			numCaps++;
		}
		if(
			bit - 3 * offset >= 0 &&
			playerGetOcc(bit - offset, opp) &&
			playerGetOcc(bit - 2 * offset, opp) &&
			playerGetOcc(bit - 3 * offset, player))
		{
			numCaps++;
		}
	}
	return numCaps;
}

int isConnect5(int bitIndex, Player_t player){
	for(int direction = 0; direction < 4; direction++){
		Offset offset = PRINCIPAL_DIRECTION[direction];
		int streak = 1;
		for(int i = 1; i < 5; i++){
			int bit = bitIndex + i * offset;
			if(playerGetOcc(bit, player)){
				streak++;
			}else{
				break;
			}
		}
		for(int i = 1; i < 5; i++){
			int bit = bitIndex - i * offset;
			if(bit >= 0 && playerGetOcc(bit, player)){
				streak++;
			}else{
				break;
			}
		}
		if(streak >= 5){
			return 1;
		}
	}
	return 0;
}



int isGameOver(int bitIndex, Player_t player){
	return g_game[g_numMoves].captures[player] == 5 || isConnect5(bitIndex, player);
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

				  Move Generation

=======================================================*/

void bbAndNotOccupied(Bitboard *b){
	Bitboard tmp = g_game[g_numMoves].occupancies[BLACK];
	bbOr(&tmp, &(g_game[g_numMoves].occupancies[WHITE]));
	bbInv(&tmp);
	bbAnd(b, &tmp);
}

// populates candidate moves
void getCandidateMoves(){
	//TODO maybe try using star2occupancies?
	Bitboard b = g_game[g_numMoves].star1Occupancies;
	bbAndNotOccupied(&b);
	int bit;
	while ( (bit = bbGetLS1B(&b)) != -1){
		unsetBit(&b, bit);	
		appendCandidateMove(bit);
	}
}

/* move value should be following order:
	1) making connect 5
	2) blocking connect 5
	3) making captures
	4) block captures
	5) playing toward center
	
	higher order moves should trump score of lower order moves
*/
#define CONNECT5_WEIGHT 		(10000)
#define CONNECT5_BLOCK_WEIGHT  	(1000)
#define CAPTURE_WEIGHT  		(100)
#define CAPTURE_BLOCK_WEIGHT	(10)
#define CENTRALIZATION_WEIGHT	(1)

// iterates over candidate moves and attatches scores
void scoreCandidateMoves(Player_t player){
	int len = g_game[g_numMoves].candidateMovesLen;
	Player_t opp = getOpp(player);
	for (int i = 0; i < len; i++){	
		int moveScore = 0;
		int bit = getCandidateMoveSquare(i);
		moveScore += CONNECT5_WEIGHT * isConnect5(bit, player);	
		moveScore += CONNECT5_BLOCK_WEIGHT * isConnect5(bit, opp);
		moveScore += CAPTURE_WEIGHT * isCapture( bit, player );
		moveScore += CAPTURE_BLOCK_WEIGHT * isCapture( bit, opp );
		moveScore += CENTRALIZATION_WEIGHT * bitDistanceFromEdgeTable[bit];
		setCandidateMoveScore(i, moveScore);	
		if(moveScore >= 10000){	
			return;
		}
	}
}


/* 
	* uses selection sort
 	* This could be replaced by quicksort for more speed
*/
void sortCandidateMoves(){
	int len  = g_game[g_numMoves].candidateMovesLen;
	for(int i = 0; i < len; i++){
		int maxIndex = i;
		int maxScore = getCandidateMoveScore(i);
		for(int k = i + 1; k < len; k++){
			int tmpScore = getCandidateMoveScore(k);
			if(tmpScore > maxScore){
				maxIndex = k;
				maxScore = tmpScore;
			}
		}
		swapCandidateMoves(i, maxIndex);
	}
}


int addMoves(int *arr, Bitboard *b){
	int i = 0;
	int bit;
	while ( (bit = bbGetLS1B(b)) != -1){
		unsetBit(b, bit);	
		arr[i++] = bit;
	}
	return i;
}



void printCandidateMoves(){
	int len = g_game[g_numMoves].candidateMovesLen;
	for(int i = 0; i < len; i++){
		Move move = getCandidateMove(i);
		printf("Score: %5d\t(%c,%d)\n", move.score, bitToFile(move.square) +'a', bitToRank(move.square));
	}
	printf("\n");
}



void bbGetConnect5Moves(Bitboard *b, Player_t player){
	memset(b, 0,  sizeof(*b));
	//set all bits that make 5 in a row.
	Bitboard star1Bb = g_game[g_numMoves].star1Occupancies;
	bbAndNotOccupied(&star1Bb);
	int bit;
	while((bit = bbGetLS1B(&star1Bb)) != -1){
		unsetBit(&star1Bb, bit);
		if(isConnect5(bit, player)){
			setBit(b, bit);
		}
	}
}

// return a bb of all moves that make or block a capture
void bbGetCaptureMoves(Bitboard *b, Player_t player){
	memset(b, 0,  sizeof(*b));
	//set all bits that make 5 in a row.
	Bitboard star1Bb = g_game[g_numMoves].star1Occupancies;
	bbAndNotOccupied(&star1Bb);
	int bit;
	while((bit = bbGetLS1B(&star1Bb)) != -1){
		unsetBit(&star1Bb, bit);
		if(isCapture(bit, player)){
			setBit(b, bit);
		}
	}
}

//TODO make a function that populates canditemoves[] with prioritized move options for player
int getMoves(Player_t player){
	int moveIndex = 0;
	int *moveArr = g_game[g_numMoves].candidateMoves;
	Bitboard cumulitiveMoves;
	memset(&cumulitiveMoves, 0, sizeof(Bitboard));
	Bitboard bitboard;

// add 5 in a row moves
	bbGetConnect5Moves( &bitboard, player);
	bbOr(&cumulitiveMoves, &bitboard);
	moveIndex += addMoves(moveArr + moveIndex, &bitboard);
// add 5 in a row blocks
	bbGetConnect5Moves(&bitboard, getOpp(player));
	bbPAndNotQ(&bitboard, &cumulitiveMoves);	//stop double adding moves
	bbOr(&cumulitiveMoves, &bitboard);	
	moveIndex += addMoves(moveArr + moveIndex, &bitboard);
// add capture moves
	bbGetCaptureMoves(&bitboard, player);
	bbPAndNotQ(&bitboard, &cumulitiveMoves);	//stop double adding moves
	bbOr(&cumulitiveMoves, &bitboard);
	moveIndex += addMoves(moveArr + moveIndex, &bitboard);
// add capture defense moves
	bbGetCaptureMoves(&bitboard, getOpp(player));
	bbPAndNotQ(&bitboard, &cumulitiveMoves);	//stop double adding moves
	bbOr(&cumulitiveMoves, &bitboard);
	moveIndex += addMoves(moveArr + moveIndex, &bitboard);
// add star 1 moves
	bitboard = g_game[g_numMoves].star1Occupancies;
	bbAndNotOccupied(&bitboard);
	bbPAndNotQ(&bitboard, &cumulitiveMoves);	//stop double adding moves
	bbOr(&cumulitiveMoves, &bitboard);
	moveIndex += addMoves(moveArr + moveIndex, &bitboard);
//add star 2 moves
/*	bbGetCaptureMoves(&bitboard, player);
	bbPAndNotQ(&bitboard, &cumulitiveMoves);	//stop double adding moves
	bbOr(&cumulitiveMoves, &bitboard);
	moveIndex += addMoves(moveArr + moveIndex, &bitboard);
*/
	return moveIndex;
}

/*=====================================================

				  AI data+functions

=======================================================*/

#define MAXIMIZER BLACK
#define MINIMIZER WHITE

typedef struct FrameStatsStruct FrameStats;
struct FrameStatsStruct{
	int pieces[2];
	int doubles[2];
	int triples[2];
	int quadruples[2];
};

FrameStats getFrameStats(){	
	FrameStats stats;
	memset(&stats, 0 , sizeof(stats));
	Bitboard maximizerBb = g_game[g_numMoves].occupancies[MAXIMIZER];
	Bitboard minimizerBb = g_game[g_numMoves].occupancies[MINIMIZER];

	stats.pieces[MAXIMIZER] = bbCountBits(&maximizerBb);
	stats.pieces[MINIMIZER] = bbCountBits(&minimizerBb);
	
	for(int direction = 0; direction < 4; direction++){
		Offset offset = PRINCIPAL_DIRECTION[direction];
		
		stats.doubles[MAXIMIZER] += bbCountOffsetStreakOfN(&maximizerBb, offset, 2);
		//printf("added %d doubles along offset %d\n", x, offset);
		stats.doubles[MINIMIZER] += bbCountOffsetStreakOfN(&minimizerBb, offset, 2);	
		stats.triples[MAXIMIZER] += bbCountOffsetStreakOfN(&maximizerBb, offset, 3);
		stats.triples[MINIMIZER] += bbCountOffsetStreakOfN(&minimizerBb, offset, 3);
		stats.quadruples[MAXIMIZER] += bbCountOffsetStreakOfN(&maximizerBb, offset, 4);
		stats.quadruples[MINIMIZER] += bbCountOffsetStreakOfN(&minimizerBb, offset, 4);
	}
	return stats;
}

const int PIECES_WEIGHT = 1000;
const int DOUBLES_WEIGHT = -800;
const int TRIPLES_WEIGHT = 100;
const int QUADRUPLES_WEIGHT = 10;
int heuristic(){
	FrameStats stats = getFrameStats();
	return 
		PIECES_WEIGHT * (stats.pieces[MAXIMIZER] - stats.pieces[MINIMIZER]) +
		DOUBLES_WEIGHT * (stats.doubles[MAXIMIZER] - stats.doubles[MINIMIZER]) +
		TRIPLES_WEIGHT * (stats.triples[MAXIMIZER] - stats.triples[MINIMIZER]) +
		QUADRUPLES_WEIGHT * (stats.quadruples[MAXIMIZER] - stats.quadruples[MINIMIZER]);
}

/* score ranges are
	heurisitic score ranges : [-500k, 500k] 
	maximizer forced win : (500k, 1 Mil)
	minimizer forced win : [-1 Mil, 500k)

	for forced whens, the number of moves until the win is calculated as 1Mil - numMoves or -1Mil + numMoves for max and min wins respectively
*/
const int MAXIMIZER_FORCED_WIN_SCORE =  1000000;
const int MINIMIZER_FORCED_WIN_SCORE = -1000000;

const int MAXIMIZER_FORCED_WIN_RANGE =  500000;	// value > than MAX_FORCED_WIN RANGE is always a forced win
const int MINIMIZER_FORCED_WIN_RANGE = -500000;	

typedef struct EvalStruct Eval;
struct EvalStruct{
	int score;
	int move;
};

int max(int x, int y){
	return x > y ? x : y;
}

int min(int x, int y){
	return x < y ? x : y;
}

long int g_nodes;	
Eval minimax(Player_t player, int depth, int alpha, int beta){
	Eval e;
	if(depth){;
		if(player == MAXIMIZER){
			// maximizer
			e.score = MINIMIZER_FORCED_WIN_SCORE-1;
			e.move = -1;
			int movesLen = getMoves(MAXIMIZER);
			for(int i = 0; i < movesLen; i++){
				int move = g_game[g_numMoves].candidateMoves[i];
				play(move, MAXIMIZER);
				if(isGameOver(move, MAXIMIZER)){
					e.score = MAXIMIZER_FORCED_WIN_SCORE;
					e.move = move;
					undo();
					return e;
				}
				Eval tmp = minimax(MINIMIZER, depth - 1, alpha, beta);
				if(tmp.score > e.score){
					e.score = tmp.score;
					e.move = move;
					if(e.score >= beta){
						undo();
						return e;
					}
				}
				if(alpha < e.score){
					alpha = e.score;
				}
				undo();
			}
			if(e.score < MINIMIZER_FORCED_WIN_RANGE){
				e.score += 1;
			}
			return e;
		}else{
			//minimizer	
			e.score = MAXIMIZER_FORCED_WIN_SCORE+1;
			e.move = -1;
			int movesLen = getMoves(MAXIMIZER);
			for(int i = 0; i < movesLen; i++){
				int move = g_game[g_numMoves].candidateMoves[i];
				play(move, MINIMIZER);
				if(isGameOver(move, MINIMIZER)){
					e.score = MINIMIZER_FORCED_WIN_SCORE;
					e.move = move;
					undo();
					return e;
				}
				Eval tmp = minimax(MAXIMIZER, depth - 1, alpha, beta);
				if(tmp.score < e.score){
					e.score = tmp.score;
					e.move = move;
					if(e.score <= alpha){
						undo();
						return e;
					}
					if(beta > e.score){
						beta = e.score;
					}
				}
				undo();
			}
			if(e.score > MAXIMIZER_FORCED_WIN_RANGE){
				e.score -= 1;
			}
			return e;
		}
	}
	g_nodes++;
	e.score = heuristic();
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
	g_nodes = 0;	
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
		g_paulFishEval = minimax(player, depth, MINIMIZER_FORCED_WIN_SCORE , MAXIMIZER_FORCED_WIN_SCORE);
		t = clock() - t;
		double timeTaken = (double)t / CLOCKS_PER_SEC;
		printf("depth:%d     score:%d     moveRec:(%c,%d)     nodes:%ld     time%lf     nodes/sec:%lf\n",depth, g_paulFishEval.score, bitToFile(g_paulFishEval.move)+'a', bitToRank(g_paulFishEval.move), g_nodes, timeTaken, g_nodes/timeTaken);
	}
}

Eval paulFish(PaulFishArgs args, int seconds){
	int retStatus; 
	pthread_t thread;
	void *res;	
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

void printMovesArr(int *arr, int n){
	for(int i = 0; i <n; i++){
		printf("(%c,%d) ", bitToFile(arr[i]) + 'a', bitToRank(arr[i]));
	}
	printf("\n");
}

void printFrameStats(FrameStats stats){
	printf("Maximizer\t\tMinimizer\n");
	printf("pieces : %3d\t\tpieces : %3d\n", stats.pieces[MAXIMIZER], stats.pieces[MINIMIZER]);	
	printf("doubles : %3d\t\tdoubles : %3d\n", stats.doubles[MAXIMIZER], stats.doubles[MINIMIZER]);	
	printf("triples : %3d\t\ttriples : %3d\n", stats.triples[MAXIMIZER], stats.triples[MINIMIZER]);	
	printf("quadruples : %3d\t\tquadruples : %3d\n", stats.quadruples[MAXIMIZER], stats.quadruples[MINIMIZER]);	
	//printf("\tsingles : %d\n\tdoubles : %d\n\ttriples : %d\n\tquadruples : %d\n", stats.singles, stats.doubles, stats.triples, stats.quadruples);
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
void testGame(){
	play(pntToBit(6,8), WHITE);
	play(pntToBit(7,6), WHITE);
	play(pntToBit(7,7), BLACK);
	play(pntToBit(7,8), BLACK);
	play(pntToBit(8,5), WHITE);
	play(pntToBit(8,6), WHITE);
	play(pntToBit(8,7), BLACK);
	play(pntToBit(8,11), BLACK);
	play(pntToBit(9,5), WHITE);
	play(pntToBit(9,7), WHITE);
	play(pntToBit(10,8), BLACK);
	play(pntToBit(11,8), BLACK);
	play(pntToBit(12,7), WHITE);
	play(pntToBit(12,6), BLACK);
	printGame();
	Eval e = minimax(BLACK, 5, MINIMIZER_FORCED_WIN_SCORE, MAXIMIZER_FORCED_WIN_SCORE);	
	printf("score %d\tmove(%c,%d)\n", e.score, bitToFile(e.move)+'a', bitToRank(e.move));
}

void testGame2(){
	play(pntToBit(6,8), WHITE);
	play(pntToBit(7,6), WHITE);
	play(pntToBit(7,7), BLACK);
	play(pntToBit(7,8), BLACK);
	play(pntToBit(8,5), WHITE);
	play(pntToBit(8,6), WHITE);
	play(pntToBit(8,7), BLACK);
	play(pntToBit(8,11), BLACK);
	play(pntToBit(9,5), WHITE);
}

void testGame3(){
	play(pntToBit(9,8), BLACK);
	play(pntToBit(8,8), BLACK);
	play(pntToBit(7,8), BLACK);
	play(pntToBit(8,7), BLACK);
	play(pntToBit(8,5), BLACK);
	play(pntToBit(11,8), WHITE);
	play(pntToBit(11,9), WHITE);
	play(pntToBit(10,9), WHITE);
	play(pntToBit(11,10), WHITE);
	printGame();	
	getCandidateMoves();
	scoreCandidateMoves(BLACK);
	sortCandidateMoves();
	printCandidateMoves();
}


int main(){	
	initGame();

//	testGame();	

	testGame3();

/*
	PaulFishArgs args;
	args.player= WHITE;
	args.maxDepth = 8;
	Eval e = paulFish(args, 45);
	printf("score %d\tmove%d\n", e.score, e.move);
	play(e.move, WHITE);
	*/

/*

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
		printGame();
		PaulFishArgs args = {player, 100};
		Eval e = paulFish(args, 20);
		printf("score %d\tmove(%c,%d)\n", e.score, bitToFile(e.move)+'a', bitToRank(e.move));
		play(e.move, player);
		if(isConnect5(e.move, player)){
			printf("GAME OVER\n");
			break;
		}
		player = getOpp(player);
	}
	printGame();
*/	



/*
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
	}
*/
}
