#include "ConnectFour.h"
#include <iostream>


ConnectFour::ConnectFour()
{
    _grid = new Grid(7, 6);
}

ConnectFour::~ConnectFour()
{
    delete _grid;
}

Bit* ConnectFour::PieceForPlayer(const int playerNumber)
{
    // depending on playerNumber load the "red.png" or the "yellow.png" graphic
    Bit *bit = new Bit();
    bit->LoadTextureFromFile(playerNumber == AI_PLAYER ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(playerNumber == AI_PLAYER ? 1 : 0));
    return bit;
}

void ConnectFour::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "square.png");

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    startGame();
    //endTurn(); // uncomment to switch turn order
}

int ConnectFour::getLowestRow(int col){
    for(int row = 5; row >= 0; row--){
        if(!_grid->getSquare(col, row)->bit()){
            return row;
        }
    }
    return -1;
}

bool ConnectFour::columnIsFull(int col){
    if(_grid->getSquare(col, 0)->bit()){
        return true;
    }
    return false;
}

bool ConnectFour::actionForEmptyHolder(BitHolder &holder)
{
    ChessSquare* clickedSquare = dynamic_cast<ChessSquare*>(&holder);
    if (holder.bit()) {
        return false;
    }
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber() == 0 ? HUMAN_PLAYER : AI_PLAYER);
    if (bit) {

        int colNum = clickedSquare->getColumn();
        int rowNum = getLowestRow(colNum);

        ChessSquare* newHolder = _grid->getSquare(colNum, rowNum);
        
        if(!columnIsFull(colNum)){ 
            bit->setPosition(holder.getPosition());
            bit->moveTo(newHolder->getPosition()); 
            newHolder->setBit(bit);
            endTurn();
            return true; 
        }
    }   
    return false;
}

bool ConnectFour::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    return false;
}

bool ConnectFour::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return false;
}

//
// free all the memory used by the game on the heap
//
void ConnectFour::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

//
// helper function for the winner check
//
Player* ConnectFour::ownerAt(int index ) const
{
    auto square = _grid->getSquare(index % 7, index / 7);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* ConnectFour::checkForWinner()
{
    for( int i=0; i<42; i++ ) { //Could change i to 39, time save would be microscpoic
        Player *player = ownerAt(i);

        //Right facing win check (3 Adjacent on the right)
        if(i % 7 <= 3){
            if(player && player == ownerAt(i+1) && player == ownerAt(i+2) && player == ownerAt(i+3)){
                return player;
            }
        }

        //Down facing win check (3 Adjacent downward)
        if(i <= 20){
            if(player && player == ownerAt(i+7) && player == ownerAt(i+14) && player == ownerAt(i+21)){
                return player;
            }
        }

        //Down and right facing win check (3 Adjacent going southeast)
        if(i <= 17 && i % 7 <= 3){
            if(player && player == ownerAt(i+8) && player == ownerAt(i+16) && player == ownerAt(i+24)){
                return player;
            }
        }

        //Down and left facing win check (3 Adjacent going southwest)
        if(i <= 20 && i % 7 >= 3){
            if(player && player == ownerAt(i+6) && player == ownerAt(i+12) && player == ownerAt(i+18)){
                return player;
            }
        }
    }
    return nullptr;
}

bool ConnectFour::checkForDraw()
{
   bool isDraw = true;
    // check to see if the board is full
    _grid->forEachSquare([&isDraw](ChessSquare* square, int x, int y) {
        if (!square->bit()) {
            isDraw = false;
        }
    });
    return isDraw;
}

//
// state strings
//
std::string ConnectFour::initialStateString()
{
    return  "0000000"
            "0000000"
            "0000000"
            "0000000"
            "0000000"
            "0000000";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string ConnectFour::stateString()
{
    std::string s = "000000000000000000000000000000000000000000";
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit *bit = square->bit();
        if (bit) {
            int playerNum = bit->getOwner()->playerNumber();
            s[y * 7 + x] = (playerNum == 0) ? '1' : '2';
        }
    });
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void ConnectFour::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*7 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber == 0 ? HUMAN_PLAYER : AI_PLAYER));
        } else {
            square->setBit( nullptr );
        }
    });
}

int AILowestRow(const std::string &s, int col){
    for(int index = 35 + col; index > -1; index-=7){
        if(s[index] == '0'){
            return index / 7; 
        }
    }
    return -1;
}
bool ConnectFour::isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}


int findSpotValue(const std::string &state){
    int Boardsum = 0;
    int pointValues[42] = { 3, 4,  5,  7,  5, 4, 3,
                            4, 6,  8, 10,  8, 6, 4,
                            5, 7, 11, 13, 11, 7, 5,
                            5, 7, 11, 13, 11, 7, 5,
                            4, 6,  8, 10,  8, 6, 4,
                            3, 4,  5,  7,  5, 4, 3,
                            };

    int index = 0;
    for(char c: state){
        if(c == '1'){
            Boardsum -= pointValues[index];
        } else if(c == '2'){
            Boardsum += pointValues[index];
        }
        index++;
    }      
    return Boardsum;
}

int ConnectFour::evaluateAIBoard(const std::string& state) {
    
    //Initialize and Award points based on grid state
    int pointSum = findSpotValue(state);

    for( int i=0; i<39; i++ ) { //Changed from 41 to 39, for minor time save
        char first = state[i];

        //Right facing win check (3 Adjacent on the right)
        if(i % 7 <= 3){
            if(first != '0' && first == state[i+1] && first == state[i+2] && first == state[i+3]){
                pointSum += (first == '2') ? 3000 : -3000;
            }
        }

        //Down facing win check (3 Adjacent downward)
        if(i <= 20){
            if(first != '0' && first == state[i+7] && first == state[i+14] && first == state[i+21]){
                pointSum += (first == '2') ? 3000 : -3000;
            }
        }

        //Down and right facing win check (3 Adjacent going southeast)
        if(i <= 17 && i % 7 <= 3){
            if(first != '0' && first == state[i+8] && first == state[i+16] && first == state[i+24]){
                pointSum += (first == '2') ? 3000 : -3000;
            }
        }

        //Down and left facing win check (3 Adjacent going southwest)
        if(i <= 20 && i % 7 >= 3){
            if(first != '0' && first == state[i+6] && first == state[i+12] && first == state[i+18]){
                pointSum += (first == '2') ? 3000 : -3000;
            }
        }
    }
    return pointSum; 
}


//
// this is the function that will be called by the AI
//
void ConnectFour::updateAI() 
{
    int initialA = -10000000;
    int initialB = 10000000;
    int bestVal = -1000000;
    BitHolder* bestMove = nullptr;
    std::string state = stateString();

    for(int col=0; col<7; col++){
        int row = AILowestRow(state, col); 
        if (row == -1) continue;// column full
        int index = row * 7 + col;
        // Check if cell is empty
        if (state[index] == '0') {
            // Make the move
            state[index] = '2';
            int moveVal = -negamax(state, 0, initialA, initialB, HUMAN_PLAYER);
            // Undo the move
            state[index] = '0';
            // If the value of the current move is more than the best value, update best
            if (moveVal > bestVal) {
                bestMove = _grid->getSquare(col, 0); // Placed at row 0 for visual affect
                bestVal = moveVal;
            }
        }
    }

    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
        }
    }
}

//
// player is the current player's number (AI or human)
//
int ConnectFour::negamax(std::string& state, int depth, int a, int b, int playerColor)
{
    
    int score = evaluateAIBoard(state);
    if(playerColor == HUMAN_PLAYER) {
        score = -score;
    }

    if (abs(score) >= 1000) return score; //Someone likely won, don't continue calculations
    if(depth == 6) return score; //Don't recurse to long

    //If board is full and no winner, DRAW
    if (isAIBoardFull(state)) {
        return 0;
    }

    int bestVal = -1000000;
    for(int col=0; col<7; col++){
        int row = AILowestRow(state, col); 
        if(row == -1){
            continue;
        }
        int index = row * 7 + col;
        if(state[index] == '0'){
            state[index] = playerColor == HUMAN_PLAYER ? '1' : '2';
            bestVal = std::max(bestVal, -negamax(state, depth+1, -b, -a, -playerColor));
            state[index] = '0';
            a = std::max(a, bestVal);
            if(a >= b){
                break;
            }
        }
    }

    //Temp bug fix, but in theory should never be true
    if (bestVal == -1000000) {
        std::cout << "Bad logic somewhere in negmax" << std::endl;
        return score; 
    }
    return bestVal;
}

void ConnectFour::setAI(bool b){
    AIGame = b;
}