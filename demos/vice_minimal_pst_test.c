#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// Minimal VICE-style definitions for PST testing
#define WHITE 0
#define BLACK 1
#define EMPTY 0
#define MAXGAMEMOVES 2048
#define OFFBOARD 100

// Piece constants
#define wP 1
#define wN 2
#define wB 3
#define wR 4
#define wQ 5
#define wK 6
#define bP 7
#define bN 8
#define bB 9
#define bR 10
#define bQ 11
#define bK 12

// Board structure (simplified)
typedef struct {
    int pieces[120];
    int side;
    int material[2];
    int pceNum[13];
    int pList[13][10];
    uint8_t castlePerm;
    int enPas;
    int fiftyMove;
    int ply;
    int hisPly;
    uint64_t posKey;
} S_BOARD;

// Conversion macros
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define MIRROR64(sq64) (Mirror64[(sq64)])

// PST Tables (from VICE evaluate.c)
const int PawnTable[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int KnightTable[64] = {
0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0		
};

const int BishopTable[64] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int RookTable[64] = {
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0		
};

const int KingO[64] = {	
	0	,	5	,	5	,	-10	,	-10	,	0	,	10	,	5	,
	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,	-30	,
	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,	-50	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,
	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70	,	-70		
};

// Conversion arrays
int Sq120ToSq64[120];
int Mirror64[64];

void InitArrays() {
    int index = 0;
    int file = 0;
    int rank = 0;
    int sq = 0;
    int sq64 = 0;
    
    // Initialize to invalid values
    for(index = 0; index < 120; ++index) {
        Sq120ToSq64[index] = 65;
    }
    
    for(index = 0; index < 64; ++index) {
        Mirror64[index] = 0;
    }
    
    // Set up 120->64 conversion
    for(rank = 0; rank < 8; ++rank) {
        for(file = 0; file < 8; ++file) {
            sq = 21 + file + rank * 10;
            Sq120ToSq64[sq] = rank * 8 + file;
        }
    }
    
    // Set up Mirror64 array
    for(index = 0; index < 64; ++index) {
        file = index % 8;
        rank = index / 8;
        Mirror64[index] = (7 - rank) * 8 + file;
    }
}

// Simple FEN parser (minimal, just for test positions)
void ParseFenSimple(const char* fen, S_BOARD* pos) {
    // Initialize board
    for(int i = 0; i < 120; ++i) pos->pieces[i] = OFFBOARD;
    for(int i = 0; i < 64; ++i) pos->pieces[21 + (i % 8) + (i / 8) * 10] = EMPTY;
    for(int i = 0; i < 13; ++i) pos->pceNum[i] = 0;
    for(int i = 0; i < 2; ++i) pos->material[i] = 0;
    
    // Material values
    int PieceVal[13] = { 0, 100, 320, 330, 500, 900, 20000, 100, 320, 330, 500, 900, 20000 };
    
    // Simple parsing - just handle the board part and side to move
    int rank = 7, file = 0;
    int sq120;
    
    while(*fen && *fen != ' ') {
        if(*fen >= '1' && *fen <= '8') {
            file += (*fen - '0');
        } else if(*fen == '/') {
            rank--;
            file = 0;
        } else {
            sq120 = 21 + file + rank * 10;
            int piece = EMPTY;
            
            switch(*fen) {
                case 'p': piece = bP; break;
                case 'r': piece = bR; break;
                case 'n': piece = bN; break;
                case 'b': piece = bB; break;
                case 'q': piece = bQ; break;
                case 'k': piece = bK; break;
                case 'P': piece = wP; break;
                case 'R': piece = wR; break;
                case 'N': piece = wN; break;
                case 'B': piece = wB; break;
                case 'Q': piece = wQ; break;
                case 'K': piece = wK; break;
            }
            
            if(piece != EMPTY) {
                pos->pieces[sq120] = piece;
                pos->pList[piece][pos->pceNum[piece]] = sq120;
                pos->pceNum[piece]++;
                
                if(piece == wP || piece == wN || piece == wB || piece == wR || piece == wQ || piece == wK) {
                    pos->material[WHITE] += PieceVal[piece];
                } else {
                    pos->material[BLACK] += PieceVal[piece];
                }
            }
            
            file++;
        }
        fen++;
    }
    
    // Skip space and get side to move
    if(*fen == ' ') fen++;
    pos->side = (*fen == 'w') ? WHITE : BLACK;
}

// Simple mirror function
void MirrorBoardSimple(S_BOARD* pos) {
    int tempPiecesArray[64];
    int tempSide = pos->side ^ 1;
    int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    
    // Store pieces in 64-board format
    for(int sq = 0; sq < 64; ++sq) {
        int sq120 = 21 + (sq % 8) + (sq / 8) * 10;
        tempPiecesArray[sq] = pos->pieces[sq120];
    }
    
    // Clear board
    for(int i = 0; i < 120; ++i) pos->pieces[i] = OFFBOARD;
    for(int i = 0; i < 64; ++i) pos->pieces[21 + (i % 8) + (i / 8) * 10] = EMPTY;
    for(int i = 0; i < 13; ++i) pos->pceNum[i] = 0;
    for(int i = 0; i < 2; ++i) pos->material[i] = 0;
    
    // Mirror and place pieces
    int PieceVal[13] = { 0, 100, 320, 330, 500, 900, 20000, 100, 320, 330, 500, 900, 20000 };
    
    for(int sq = 0; sq < 64; ++sq) {
        int mirrored_sq = Mirror64[sq];
        int piece = SwapPiece[tempPiecesArray[mirrored_sq]];
        
        if(piece != EMPTY) {
            int sq120 = 21 + (sq % 8) + (sq / 8) * 10;
            pos->pieces[sq120] = piece;
            pos->pList[piece][pos->pceNum[piece]] = sq120;
            pos->pceNum[piece]++;
            
            if(piece == wP || piece == wN || piece == wB || piece == wR || piece == wQ || piece == wK) {
                pos->material[WHITE] += PieceVal[piece];
            } else {
                pos->material[BLACK] += PieceVal[piece];
            }
        }
    }
    
    pos->side = tempSide;
}

// Simple evaluation: Material + PST only (absolute score from White's perspective)
int SimpleEvalAbsolute(const S_BOARD *pos) {
    int score = pos->material[WHITE] - pos->material[BLACK];
    
    // White pawns
    for(int i = 0; i < pos->pceNum[wP]; ++i) {
        int sq = pos->pList[wP][i];
        score += PawnTable[SQ64(sq)];    
    }    

    // Black pawns  
    for(int i = 0; i < pos->pceNum[bP]; ++i) {
        int sq = pos->pList[bP][i];
        score -= PawnTable[MIRROR64(SQ64(sq))];    
    }    
    
    // White knights
    for(int i = 0; i < pos->pceNum[wN]; ++i) {
        int sq = pos->pList[wN][i];
        score += KnightTable[SQ64(sq)];
    }    

    // Black knights
    for(int i = 0; i < pos->pceNum[bN]; ++i) {
        int sq = pos->pList[bN][i];
        score -= KnightTable[MIRROR64(SQ64(sq))];
    }            
    
    // White bishops
    for(int i = 0; i < pos->pceNum[wB]; ++i) {
        int sq = pos->pList[wB][i];
        score += BishopTable[SQ64(sq)];
    }    

    // Black bishops
    for(int i = 0; i < pos->pceNum[bB]; ++i) {
        int sq = pos->pList[bB][i];
        score -= BishopTable[MIRROR64(SQ64(sq))];
    }    

    // White rooks
    for(int i = 0; i < pos->pceNum[wR]; ++i) {
        int sq = pos->pList[wR][i];
        score += RookTable[SQ64(sq)];
    }    

    // Black rooks
    for(int i = 0; i < pos->pceNum[bR]; ++i) {
        int sq = pos->pList[bR][i];
        score -= RookTable[MIRROR64(SQ64(sq))];
    }    
    
    // White king
    if(pos->pceNum[wK] > 0) {
        int sq = pos->pList[wK][0];
        score += KingO[SQ64(sq)];
    }
    
    // Black king
    if(pos->pceNum[bK] > 0) {
        int sq = pos->pList[bK][0];
        score -= KingO[MIRROR64(SQ64(sq))];
    }
    
    return score; // Absolute score from White's perspective
}

// Simple evaluation with side-to-move perspective (for actual gameplay)
int SimpleEvalSideToMove(const S_BOARD *pos) {
    int absolute_score = SimpleEvalAbsolute(pos);
    
    // Return from side-to-move perspective
    return (pos->side == WHITE) ? absolute_score : -absolute_score;
}

void PrintBoard(const S_BOARD* pos) {
    char PceChar[] = ".PNBRQKpnbrqk";
    printf("\\nBoard:\\n");
    
    for(int rank = 7; rank >= 0; rank--) {
        printf("%d  ", rank + 1);
        for(int file = 0; file < 8; file++) {
            int sq120 = 21 + file + rank * 10;
            int piece = pos->pieces[sq120];
            printf("%c ", PceChar[piece]);
        }
        printf("\\n");
    }
    
    printf("   ");
    for(int file = 0; file < 8; file++) {
        printf("%c ", 'a' + file);
    }
    printf("\\nSide: %s\\n", pos->side == WHITE ? "White" : "Black");
    printf("Material: White=%d, Black=%d\\n", pos->material[WHITE], pos->material[BLACK]);
}

int main() {
    InitArrays();
    
    printf("VICE-Style Simple Evaluation Test (Material + PST only)\\n");
    printf("======================================================\\n\\n");
    
    S_BOARD pos;
    
    // Test VICE e4-c6 position
    printf("### Testing VICE e4-c6 Position ###\\n");
    ParseFenSimple("rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2", &pos);
    
    printf("Original position:\\n");
    PrintBoard(&pos);
    
    int original_absolute = SimpleEvalAbsolute(&pos);
    int original_side_to_move = SimpleEvalSideToMove(&pos);
    
    printf("\\nEvaluation (original):\\n");
    printf("  Absolute score: %d (White's perspective)\\n", original_absolute);
    printf("  Side-to-move score: %d (%s's perspective)\\n", 
           original_side_to_move, pos.side == WHITE ? "White" : "Black");
    
    // Mirror and test
    printf("\\nMirroring position...\\n");
    MirrorBoardSimple(&pos);
    
    printf("Mirrored position:\\n");
    PrintBoard(&pos);
    
    int mirrored_absolute = SimpleEvalAbsolute(&pos);
    int mirrored_side_to_move = SimpleEvalSideToMove(&pos);
    
    printf("\\nEvaluation (mirrored):\\n");
    printf("  Absolute score: %d (White's perspective)\\n", mirrored_absolute);
    printf("  Side-to-move score: %d (%s's perspective)\\n", 
           mirrored_side_to_move, pos.side == WHITE ? "White" : "Black");
    
    printf("\\n=== SYMMETRY ANALYSIS ===\\n");
    printf("Absolute scores:\\n");
    printf("  Original: %d\\n", original_absolute);
    printf("  Mirrored: %d\\n", mirrored_absolute);
    printf("  Sum (should be 0): %d\\n", original_absolute + mirrored_absolute);
    
    printf("\\nSide-to-move scores:\\n");
    printf("  Original: %d\\n", original_side_to_move);
    printf("  Mirrored: %d\\n", mirrored_side_to_move);
    printf("  Sum: %d\\n", original_side_to_move + mirrored_side_to_move);
    
    if (original_absolute + mirrored_absolute == 0) {
        printf("\\n✅ Perfect symmetry achieved!\\n");
        printf("   PST tables are symmetric and mirror function works correctly.\\n");
        
        if (original_side_to_move > 0 && mirrored_side_to_move > 0) {
            printf("   Side-to-move perspective: Both sides see advantage (correct for gameplay)\\n");
        }
    } else {
        printf("\\n❌ Asymmetry detected: %d points\\n", abs(original_absolute + mirrored_absolute));
        printf("   This suggests PST tables are not perfectly symmetric\\n");
        printf("   or there's a bug in the mirroring function.\\n");
    }
    
    return 0;
}
