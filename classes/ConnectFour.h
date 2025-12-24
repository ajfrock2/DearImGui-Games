#pragma once
#include "Game.h"

//
// the main game class
//
class ConnectFour : public Game
{
public:
    ConnectFour();
    ~ConnectFour();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void        stopGame() override;

	void        updateAI() override;
    bool        gameHasAI() override { return AIGame; }
    Grid*       getGrid() override { return _grid; }
    void        setAI(bool b);
    bool        AIGame = true;
private:
    Bit *       PieceForPlayer(const int playerNumber);
    Player*     ownerAt(int index ) const;
    int         negamax(std::string& state, int depth, int a, int b, int playerColor);
    int         evaluateAIBoard(const std::string& state);
    bool        isAIBoardFull(const std::string& state);
    int         getLowestRow(int col);  
    bool        columnIsFull(int col);
    Grid*       _grid;
};

