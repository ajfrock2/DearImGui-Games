#pragma once
#include "Game.h"

//
// the main game class
//
class CatchingStrays : public Game
{
public:
    CatchingStrays();
    ~CatchingStrays();

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
    bool        canTranslate(BitHolder &src, BitHolder &dst);
    void        updateAI() override;
    bool        gameHasAI() override { return true; }
    int         getLowestRow(int col);
    void        checkNearbyFruit(BitHolder &holder);

    Grid*       getGrid() override { return _grid; }
private:
    Bit *       PieceForPlayer(const int playerNumber);
    Player*     ownerAt(int index ) const;
    Grid*       _grid;
};

