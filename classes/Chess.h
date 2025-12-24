#pragma once

#include <array>
#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"
#include "MagicBitboards.h"
#include "GameState.h"

constexpr int pieceSize = 80;

/*
enum ChessPiece
{
    NoPiece,
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};
*/

class Chess : public Game
{
public:
    Chess();
    ~Chess();

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;
    void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;


    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;

    Grid* getGrid() override { return _grid; }

    void initializeStaticMoves();
    void findValidMoves(std::vector<BitMove> &validMoves, int playerColor, std::string state);
    void findPiece(BitboardElement &PosBitboard, Bit* piece, int index, int tag);
    void addStaticMoves(BitboardElement (&staticPieceMoves)[64], const std::pair<int,int> offsets[], int numOffsets, int i, int x, int y);
    void generateKnightMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves);
    void generateKingMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves, int playerColor);
    void generatePawnMoves(BitboardElement positionBitboard, BitboardElement enemyPositionBitboard, BitboardElement occupancyBitboard, 
        std::vector<BitMove> &validMoves, bool white);
    void generateRookMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves);
    void generateBishopMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves);
    void generateQueenMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves);
    void updateAI() override;
    int  negamax(std::string& state, int depth, int a, int b, int playerColor, int moveCount);
    bool gameHasAI() override { return true; }
    int evaluateBoard(const std::string& state, int moveCount);
    void endTurn() override;
    void makeMove(BitMove &move);

    // Tournament support methods
    void setBoardFromFEN(const std::string& fen);
    BitMove getLastAIMove() const { return _lastAIMove; }
    std::string getFEN() const;

    // Get current player color (WHITE=1, BLACK=-1)
    int getCurrentPlayerColor() const { return _currentPlayer; }

    // you can make this variable private, it's just grouped with the public methods for convenience
    BitMove _lastAIMove;  // Stores the last move calculated by AI (for tournament)
    GameState _gamestate;
    int _currentPlayer;


private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;

    Grid* _grid;
    BitboardElement staticKnightMoves[64];
    BitboardElement staticKingMoves[64];
    std::vector<BitMove> allValidMoves;

    std::array<std::array<int, 64>, 256> combinedScore = []{
        std::array<std::array<int, 64>, 256> arr{};

        // --- Piece values ---
        const int P = 100, N = 300, B = 300, R = 500, Q = 900, K = 2000;

        // Stockfish-style PST

        // Pawn
        const int pawnPST[64] = {
            0,  0,  0,  0,  0,  0,  0,  0,
            50, 50, 50, 50, 50, 50, 50, 50,
            10, 10, 20, 30, 30, 20, 10, 10,
            5,  5, 10, 25, 25, 10,  5,  5,
            0,  0,  0, 20, 20,  0,  0,  0,
            5, -5,-10,  0,  0,-10, -5,  5,
            5, 10, 10,-20,-20, 10, 10,  5,
            0,  0,  0,  0,  0,  0,  0,  0
        };

        // Knight
        const int knightPST[64] = {
            -50,-40,-30,-30,-30,-30,-40,-50,
            -40,-20,  0,  5,  5,  0,-20,-40,
            -30,  5, 10, 15, 15, 10,  5,-30,
            -30,  0, 15, 20, 20, 15,  0,-30,
            -30,  5, 15, 20, 20, 15,  5,-30,
            -30,  0, 10, 15, 15, 10,  0,-30,
            -40,-20,  0,  0,  0,  0,-20,-40,
            -50,-40,-30,-30,-30,-30,-40,-50
        };

        // Bishop
        const int bishopPST[64] = {
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -20,-10,-10,-10,-10,-10,-10,-20
        };

        // Rook
        const int rookPST[64] = {
            0,  0,  0,  0,  0,  0,  0,  0,
            5, 10, 10, 10, 10, 10, 10,  5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            0,  0,  0,  5,  5,  0,  0,  0
        };

        // Queen
        const int queenPST[64] = {
            -20,-10,-10, -5, -5,-10,-10,-20,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -10,  0,  5,  5,  5,  5,  0,-10,
            -5,  0,  5,  5,  5,  5,  0, -5,
            0,  0,  5,  5,  5,  5,  0, -5,
            -10,  5,  5,  5,  5,  5,  0,-10,
            -10,  0,  5,  0,  0,  0,  0,-10,
            -20,-10,-10, -5, -5,-10,-10,-20
        };

        // King
        const int kingPST[64] = {
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            20, 20,  0,  0,  0,  0, 20, 20,
            20, 30, 10,  0,  0, 10, 30, 20
        };

        auto fill = [&](char white, char black, int value, const int pst[64]) {
            for (int sq = 0; sq < 64; sq++) {
                arr[(int)white][sq] = value + pst[sq];
                arr[(int)black][sq] = -value - pst[63 - sq];
            }
        };

        // Fill all pieces
        fill('P', 'p', P, pawnPST);
        fill('N', 'n', N, knightPST);
        fill('B', 'b', B, bishopPST);
        fill('R', 'r', R, rookPST);
        fill('Q', 'q', Q, queenPST);
        fill('K', 'k', K, kingPST);

        // Empty square
        for (int sq = 0; sq < 64; sq++) {
            arr['0'][sq] = 0;
        }
        return arr;
    }();


    uint8_t whiteFlags = 0;
    uint8_t blackFlags = 0;

    int negaCount;
};