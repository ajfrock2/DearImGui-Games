#include "CatchingStrays.h"
#include <iostream>
#include <random>
#include <cmath> 

const int playerID = 1;
const int treeID = 2;
const int cherryID = 3;
const int appleID = 4;
const int orangeID = 5;
Bit* Playerbit = nullptr;


CatchingStrays::CatchingStrays()
{
    _grid = new Grid(15, 8);
}

CatchingStrays::~CatchingStrays()
{
    delete _grid;
    delete Playerbit;
}

Bit* CatchingStrays::PieceForPlayer(const int playerNumber)
{
    Bit *bit = new Bit();
    switch(playerNumber){
        case playerID:
            bit->LoadTextureFromFile("Girl100.png");
            break;
        case treeID:
            bit->LoadTextureFromFile("Tree100.png");
            break; 
        case cherryID: 
            bit->LoadTextureFromFile("Cherry100.png");
            bit->setGameTag(3);
            break; 
        case appleID:
            bit->LoadTextureFromFile("Apple100.png");
            bit->setGameTag(1);
            break; 
        case orangeID:
            bit->LoadTextureFromFile("Orange100.png");
            bit->setGameTag(2);
            break; 
        default:
            break;
    }
    bit->setOwner(getPlayerAt(0));
    return bit;
}

void CatchingStrays::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 15;
    _gameOptions.rowY = 8;
    setScore(0);
    _grid->initializeSquares(80, "Border100.png");

    //Setting the player
    Playerbit = PieceForPlayer(1);
    BitHolder* startHolder = _grid->getSquare(7, 7);
    startHolder->setBit(Playerbit);
    Playerbit->setParent(startHolder);
    Playerbit->setPosition(startHolder->getPosition());

    //Setting trees
    for(int i = 0; i<15; i++){
        Bit* treebit = PieceForPlayer(2);
        BitHolder* tempHolder = _grid->getSquare(i, 0);
        tempHolder->setBit(treebit);
        treebit->setParent(tempHolder);
        treebit->setPosition(tempHolder->getPosition());
    }

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }
    startGame();
}

//Function checking if two holders share the same y position and adjacent x, used for girl horizontal movement
bool CatchingStrays::canTranslate(BitHolder &src, BitHolder &dst){
    ChessSquare* sourceSqaure = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* destSquare = dynamic_cast<ChessSquare*>(&dst);

    return (destSquare->getRow() == sourceSqaure->getRow() 
    && (std::abs(destSquare->getColumn() - sourceSqaure->getColumn())) == 1);
}

//Move the player horizontally
bool CatchingStrays::actionForEmptyHolder(BitHolder &holder)
{
    if(canTranslate(*Playerbit->getHolder(), holder)){
        Playerbit->moveTo(holder.getPosition());
        holder.setBit(Playerbit);
        checkNearbyFruit(holder);
        endTurn();
        return true;
    }
    return false;
    
}

//Checks if the there is a fruit above the holder and increase score if that is the case
void CatchingStrays::checkNearbyFruit(BitHolder &holder){
    ChessSquare* clickedSquare = dynamic_cast<ChessSquare*>(&holder);
    int colNum = clickedSquare->getColumn();
    int rowNum = clickedSquare->getRow();

    Bit* fruitBit = _grid->getSquare(colNum, rowNum-1)->bit();
    if(fruitBit){
        setScore( getScore() + fruitBit->gameTag()); //Game tag is score value
        _grid->getSquare(colNum, rowNum-1)->destroyBit();
    }
}

bool CatchingStrays::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    return true;
}

bool CatchingStrays::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return false;
}

//
// free all the memory used by the game on the heap
//
void CatchingStrays::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

//
// helper function for the winner check
//
Player* CatchingStrays::ownerAt(int index ) const
{
    return nullptr;
}

Player* CatchingStrays::checkForWinner()
{
    return nullptr;
}

bool CatchingStrays::checkForDraw()
{
    return false;
}

//
// state strings
//
std::string CatchingStrays::initialStateString()
{
    return  "000000000000000"
            "000000000000000"
            "000000000000000"
            "000000000000000"
            "000000000000000"
            "000000000000000"
            "000000000000000"
            "000000000000000";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string CatchingStrays::stateString()
{
    std::string s(120, '0');
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit *bit = square->bit();
        if (bit) {
            int playerNum = bit->getOwner()->playerNumber();
            s[y * 15 + x] = playerNum + '0';
        }
    });
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void CatchingStrays::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*15 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber));
        } else {
            square->setBit( nullptr );
        }
    });
}

int CatchingStrays::getLowestRow(int col){
    for(int row = 7; row >= 0; row--){
        if(!_grid->getSquare(col, row)->bit()){
            return row;
        }
    }
    return -1;
}

void CatchingStrays::updateAI(){

    std::random_device rng;
    std::mt19937 gen(rng());
    std::uniform_int_distribution<> locDistrib(0, 14);
    std::uniform_int_distribution<> fruitDistrib(3, 5);
    std::uniform_int_distribution<> spawnDistrib(0, 4);

    //Delete old fruits
    for(int i=90; i<105; i++){
        BitHolder* currHolder = _grid->getSquareByIndex(i);
        currHolder->destroyBit();
    }    

    //Move fruits
    for(int i=89; i>=15; i--){
        BitHolder* currHolder = _grid->getSquareByIndex(i);
        Bit* tempBit = currHolder->bit();
        
        if(tempBit){
            ChessSquare* newHolder = _grid->getSquareByIndex(i+15);
            tempBit->moveTo(newHolder->getPosition());
            newHolder->setBit(tempBit);
        }
    }

    //Make more fruits
    for(int i = 0; i<spawnDistrib(gen); i++){
        int randPos = locDistrib(gen);
        BitHolder* topHolder = _grid->getSquare(randPos, 1);
        Bit* fruitBit = PieceForPlayer(fruitDistrib(gen));

        topHolder->setBit(fruitBit);
        fruitBit->setPosition(topHolder->getPosition());
    }
    endTurn();
}