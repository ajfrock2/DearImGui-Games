#pragma once

#include <assert.h>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <vector>
#include "Bitboard.h"

constexpr int WHITE = +1;
constexpr int BLACK = -1;
// Define a constant for the maximum depth of your AI.
constexpr int MAX_DEPTH = 24;
// Define constants for ranks and files
constexpr uint64_t NotAFile(0xFEFEFEFEFEFEFEFEULL); // A file mask
constexpr uint64_t NotHFile(0x7F7F7F7F7F7F7F7FULL); // H file mask
constexpr uint64_t Rank3(0x0000000000FF0000ULL); // Rank 3 mask
constexpr uint64_t Rank6(0x0000FF0000000000ULL); // Rank 6 mask

enum AllBitBoards
{
    WHITE_PAWNS,
    WHITE_KNIGHTS,
    WHITE_BISHOPS,
    WHITE_ROOKS,
    WHITE_QUEENS,
    WHITE_KING,
    WHITE_ALL_PIECES,
    BLACK_PAWNS,
    BLACK_KNIGHTS,
    BLACK_BISHOPS,
    BLACK_ROOKS,
    BLACK_QUEENS,
    BLACK_KING,
    BLACK_ALL_PIECES,
    OCCUPANCY,
    EMPTY_SQUARES,
    e_numBitboards
};

enum MoveFlags {
    EnPassant = 0x01, // 0000 0001
    IsCapture = 0x02, // 0000 0010
    KingSideCastle = 0x04, // 0000 0100
    QueenSideCastle = 0x08, // 0000 1000
    IsPromotion = 0x10 // 0001 0000
};


#pragma pack(pop)

struct alignas(32) GameStateData {
    char state[64];                 // persisitent
    int flags;
    char color;                     // BLACK or WHITE

    GameStateData() : flags(0)
        , color(WHITE) {
        std::memset(state, '0', sizeof(state));
    }
    GameStateData(const GameStateData&) = default;
    GameStateData& operator=(const GameStateData&) = default;
};

class GameState : public GameStateData {
public:
    GameStateData stateStack[MAX_DEPTH];
    int stackPtr = 0;

    uint64_t _zobristHash[2]; // when one hash value is made, the other is made as well because it's just a xor of the first by the color bit
    BitboardElement _bitboards[e_numBitboards];
    BitboardElement _attackBitBoard;

    GameState() : stackPtr(0) { }

    void init(const char* newState, char player);

    inline void pushMove(const BitMove& move) {
        pushState();
        unsigned char fromPiece = state[move.from];
        state[move.from] = '0';
        state[move.to] = fromPiece;
        if (move.flags & KingSideCastle) {
            state[move.to - 1] = state[move.to + 1];
            state[move.to + 1] = '0';
        } else if (move.flags & QueenSideCastle) {
            state[move.to + 1] = state[move.to - 2];
            state[move.to - 2] = '0';
        } else if (move.flags & EnPassant) {
            // check for color to determine which direction to capture
            if (fromPiece == 'P') {
                state[move.to - 8] = '0';
            } else {
                state[move.to + 8] = '0';
            }
        } else if (move.flags & IsPromotion) {
            state[move.to] = color == WHITE ? 'Q' : 'q';
        }
        // flip the color bit as it now becomes the other player's turn
        color = (color == WHITE) ? BLACK : WHITE;
        flags = 0; // invalidate all the flags
    }

    inline void pushState() {
        assert(stackPtr < MAX_DEPTH);
        stateStack[stackPtr++] = static_cast<const GameStateData&>(*this);
    }
    inline void popState() {
        assert(stackPtr > 0);
        static_cast<GameStateData&>(*this) = stateStack[--stackPtr];
    }

    std::vector<BitMove> generateAllMoves();
    void shutdown();
    void filterOutIllegalMoves(std::vector<BitMove>& moves);
private:
    const BitboardElement generatePawnAttacks(const BitboardElement pawns, char color);
    uint64_t generatePawnAttacksBitBoard(int square, char color);
    
    void generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t occupancy);
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t occupancy);
    void generateRooksMoves(std::vector<BitMove>& moves, BitboardElement bishopBoard, uint64_t occupancy, uint64_t friendlies);
    void generateQueensMoves(std::vector<BitMove>& moves, BitboardElement bishopBoard, uint64_t occupancy, uint64_t friendlies);

    void generateBishopMoves(std::vector<BitMove>& moves, BitboardElement bishopBoard, uint64_t occupancy, uint64_t friendlies);
    void generatePawnMoveList(std::vector<BitMove>& moves, const BitboardElement pawns, const BitboardElement emptySquares, const BitboardElement enemyPieces, char color);
    void addPawnBitboardMovesToList(std::vector<BitMove>& moves, BitboardElement  bitboard, const int shift);
    bool isSquareAttacked(int square, char attackerColor, const BitboardElement (&boards)[e_numBitboards]);

};
