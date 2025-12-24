#include "Chess.h"
#include <limits>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    initializeStaticMoves();

    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"); // Vanilla
    //FENtoBoard("8/1p3pp1/p2p4/3P4/4P3/5N2/PP3PPP/RNBQKB1R"); // Test 1
    //FENtoBoard("r1bqkbnr/pppppppp/2n5/8/8/2N5/PPPPPPPP/R1BQKBNR"); // Test 2
    //FENtoBoard("rnbqkbnr/pppppppp/2k5/8/8/5K2/PPPPPPPP/RNBQKBNR"); // King Test
    //FENtoBoard("8/8/1R4r1/8/1r4R1/8/8/8"); // Rook test
    //FENtoBoard("8/8/1B4b1/8/1b4B1/8/8/8"); // Bishop test
    //FENtoBoard("8/8/1Q4q1/8/1q4Q1/8/8/8"); // Queen test
    //FENtoBoard("8/8/4n3/8/3P1P2/Q7/8/8"); //AI Test
    //FENtoBoard("8/8/2pp4/8/8/8/8/8");

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }
    _currentPlayer = 1;

    negaCount = 0;
    allValidMoves.clear();
    initMagicBitboards();
    findValidMoves(allValidMoves, getCurrentPlayer()->playerNumber(), stateString());
    startGame();
}

// Takes in a fen string and modifies the board to match it 
// ------------------------------TODO expand logic for task 2,3,4,5------------------------------

// convert a FEN string to a board
// FEN is a space delimited string with 6 fields
// 1: piece placement (from white's perspective)
// NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
// ARE BELOW
// 2: active color (W or B)
// 3: castling availability (KQkq or -)
// 4: en passant target square (in algebraic notation, or -)
// 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
void Chess::FENtoBoard(const std::string& fen) 
{
    // Loop over all 64 squares and remove pieces
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            BitHolder* sq = _grid->getSquare(file, rank);
            Bit* piece = sq->bit();
            if (piece) {
                sq->setBit(nullptr);
            }
        }
    }
    

    // Parsing into 8 strings
    std::vector<std::string> rows;
    std::stringstream ss(fen);
    std::string item;
    while (std::getline(ss, item, '/')) {
        rows.push_back(item);
    }

    int rank = 0;
    int file = 0;
    // Iterate over each letter correlating to different square
    for(int i = 7; i>=0; i--){
        std::string letters = rows[i];
        for(char letter: letters){
            Bit* piece;
            int tag = 0;
            int playerColor = 1; // Default Black
            int asciiVal = static_cast<int> (letter);

            // If uppercase, change to white
            if(asciiVal >= 65 && asciiVal <= 90){
                playerColor = 0;
                letter = std::tolower(letter);
            }

            // Get piece based on char and player number
            switch(letter)
            {
                case 'p':
                    piece = PieceForPlayer(playerColor, ChessPiece::Pawn);
                    tag = 1;
                    break;
                case 'r':
                    piece = PieceForPlayer(playerColor, ChessPiece::Rook);
                    tag = 4;
                    break;
                case 'n':
                    piece = PieceForPlayer(playerColor, ChessPiece::Knight);
                    tag = 2;
                    break;
                case 'b':
                    piece = PieceForPlayer(playerColor, ChessPiece::Bishop);
                    tag = 3;
                    break;
                case 'k':
                    piece = PieceForPlayer(playerColor, ChessPiece::King);
                    tag = 6;
                    break;
                case 'q':
                    piece = PieceForPlayer(playerColor, ChessPiece::Queen);
                    tag = 5;
                    break;
                default:
                    // Number logic

                    int num = letter - '0';
                    file+=num;
                    continue;
            }
            // Add the piece to board
            piece->setPosition(_grid->getSquare(file, rank)->getPosition());
            _grid->getSquare(file, rank)->setBit(piece);
            piece->setGameTag(tag + 128 * playerColor);
            
            file++;
        }
        rank++;
        file=0;
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    for(BitMove move: allValidMoves){
        if(_grid->getSquareByIndex(move.from) == &src
        && _grid->getSquareByIndex(move.to) == &dst){
            return true;
        }
    }
    return false;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    Game::bitMovedFromTo(bit, src, dst);
    
    // Set castle flags
    if(_grid->getSquareByIndex(0) == &src){
        whiteFlags |= QueenSideCastle;
    }
    if(_grid->getSquareByIndex(7) == &src){
        whiteFlags |= KingSideCastle;
    }
    if(_grid->getSquareByIndex(56) == &src){
        blackFlags |= QueenSideCastle;
    }
    if(_grid->getSquareByIndex(63) == &src){
        blackFlags |= KingSideCastle;
    }

    //White castle check extra move
    if(_grid->getSquareByIndex(4) == &src){
        // Disable castling for white
        whiteFlags |= KingSideCastle | QueenSideCastle;

        // King Side
        if(_grid->getSquareByIndex(6) == &dst){
            BitMove move = BitMove(7, 5, Rook);
            makeMove(move);
            
        }
        // Queen Side
        if(_grid->getSquareByIndex(2) == &dst){
            BitMove move = BitMove(0, 3, Rook);
            makeMove(move);
        }
    }

    //Black castle check extra move
    if(_grid->getSquareByIndex(60) == &src){
        // Disable castling for black
        blackFlags |= KingSideCastle | QueenSideCastle;

        // King Side
        if(_grid->getSquareByIndex(62) == &dst){
            BitMove move = BitMove(63, 61, Rook);
            makeMove(move);
        }
        // Queen Side
        if(_grid->getSquareByIndex(58) == &dst){
            BitMove move = BitMove(56, 59, Rook);
            makeMove(move);
        }
    }
}

void Chess::endTurn()
{
    Game::endTurn();
    _currentPlayer *= -1;
    allValidMoves.clear();
    findValidMoves(allValidMoves, getCurrentPlayer()->playerNumber(), stateString());
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

// Find all the static moves for Knights and Kings
void Chess::initializeStaticMoves(){
    std::pair<int, int> knightOffsets[8] = {{1,2}, {1,-2}, {-1,2}, {-1,-2}, {2,1}, {2,-1}, {-2,1}, {-2,-1}};
    std::pair<int, int> kingOffsets[8] = {{1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}};
    for(int i=0; i<64; i++){
        int x = i % 8;  // column/file
        int y = i / 8;  // row/rank

        addStaticMoves(staticKnightMoves, knightOffsets, 8, i, x, y);
        addStaticMoves(staticKingMoves, kingOffsets, 8, i, x, y);
    }
}
// Add static moves to the each square array based on offsets
void Chess::addStaticMoves(BitboardElement (&staticPieceMoves)[64], const std::pair<int,int> offsets[], int numOffsets, int i, int x, int y){
    staticPieceMoves[i] = 0ULL;
    for(int j=0; j<numOffsets; j++){
        std::pair<int, int> offset = offsets[j];
            int newX = x + offset.first;
            int newY = y + offset.second;

            // Check for no off board moves
            if(newX < 8 && newX > -1 && newY < 8 && newY >-1){
                staticPieceMoves[i] |= (1ULL << (newX + 8 * newY));
            }
        }
}

// Finds all valid chess moves from the board state
void Chess::findValidMoves(std::vector<BitMove> &validMoves, int playerColor, std::string state){
    BitboardElement whiteKnightPosBitboard = 0;
    BitboardElement blackKnightPosBitboard = 0;
    BitboardElement whiteKingPosBitboard = 0;
    BitboardElement blackKingPosBitboard = 0;
    BitboardElement whitePawnPosBitboard = 0;
    BitboardElement blackPawnPosBitboard = 0;
    BitboardElement whiteQueenPosBitboard = 0;
    BitboardElement blackQueenPosBitboard = 0;
    BitboardElement whiteRookPosBitboard = 0;
    BitboardElement blackRookPosBitboard = 0;
    BitboardElement whiteBishopPosBitboard = 0;
    BitboardElement blackBishopPosBitboard = 0;

    BitboardElement whitePosBitboard = 0;
    BitboardElement blackPosBitboard = 0;
    BitboardElement occupancyBitboard = 0;
    if(playerColor == 0){
        playerColor = -1;
    }

    //Find all pieces and update positionBitBoards
    for (int i = 0; i < 64 && i < (int)state.size(); i++) {
        char c = state[i];
        if (c == '0') continue;

        switch (c) {
            // White Pieces
            case 'P':  // White Pawn
                whitePawnPosBitboard |= (1ULL << i);
                break;
            case 'N':  // White Knight
                whiteKnightPosBitboard |= (1ULL << i);
                break;
            case 'B':  // White Bishop
                whiteBishopPosBitboard |= (1ULL << i);
                break;
            case 'R':  // White Rook
                whiteRookPosBitboard |= (1ULL << i);
                break;
            case 'Q':  // White Queen
                whiteQueenPosBitboard |= (1ULL << i);
                break;
            case 'K':  // White King
                whiteKingPosBitboard |= (1ULL << i);
                break;

            // Black Pieces
            case 'p':  // Black Pawn
                blackPawnPosBitboard |= (1ULL << i);
                break;
            case 'n':  // Black Knight
                blackKnightPosBitboard |= (1ULL << i);
                break;
            case 'b':  // Black Bishop
                blackBishopPosBitboard |= (1ULL << i);
                break;
            case 'r':  // Black Rook
                blackRookPosBitboard |= (1ULL << i);
                break;
            case 'q':  // Black Queen
                blackQueenPosBitboard |= (1ULL << i);
                break;
            case 'k':  // Black King
                blackKingPosBitboard |= (1ULL << i);
                break;
            default:
                break;
        }
    }


    whitePosBitboard = whiteKnightPosBitboard | whiteKingPosBitboard | whitePawnPosBitboard | 
        whiteBishopPosBitboard | whiteRookPosBitboard | whiteQueenPosBitboard;
    blackPosBitboard = blackKnightPosBitboard | blackKingPosBitboard | blackPawnPosBitboard | 
        blackBishopPosBitboard | blackRookPosBitboard | blackQueenPosBitboard;
    occupancyBitboard = whitePosBitboard | blackPosBitboard;
    
    // White moves
    if(playerColor == -1)
    {
        generateKnightMoves(whiteKnightPosBitboard, whitePosBitboard, validMoves);
        generatePawnMoves(whitePawnPosBitboard, blackPosBitboard, occupancyBitboard, validMoves, true);
        generateQueenMoves(whiteQueenPosBitboard, whitePosBitboard, occupancyBitboard, validMoves);
        generateBishopMoves(whiteBishopPosBitboard, whitePosBitboard, occupancyBitboard, validMoves);
        generateRookMoves(whiteRookPosBitboard, whitePosBitboard, occupancyBitboard, validMoves);
        generateKingMoves(whiteKingPosBitboard, whitePosBitboard, validMoves, playerColor);
    }
    // Black Moves
    else
    {
        generateKnightMoves(blackKnightPosBitboard, blackPosBitboard, validMoves);
        generatePawnMoves(blackPawnPosBitboard, whitePosBitboard, occupancyBitboard, validMoves, false);
        generateQueenMoves(blackQueenPosBitboard, blackPosBitboard, occupancyBitboard, validMoves);
        generateBishopMoves(blackBishopPosBitboard, blackPosBitboard, occupancyBitboard, validMoves);
        generateRookMoves(blackRookPosBitboard, blackPosBitboard, occupancyBitboard, validMoves);
        generateKingMoves(blackKingPosBitboard, blackPosBitboard, validMoves, playerColor);

    }

}

// Looks at a piece, it if matches the tag, adds it to the bitboard
void Chess::findPiece(BitboardElement &PosBitboard, Bit* piece, int index, int tag){
    if(piece->gameTag() == tag){
    // Add both piece colors to bitboard based on stateString
        PosBitboard |= (1ULL << index);   
    }
}

void Chess::generateKnightMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement tempOffets = staticKnightMoves[indexOfFrom];
        tempOffets = tempOffets & (~friendlyBitboard);
        tempOffets.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Knight);
        });
    });
}

void Chess::generateKingMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, std::vector<BitMove> &validMoves, int playerColor){
    uint8_t flagsToEnable = KingSideCastle | QueenSideCastle;
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement tempOffets = staticKingMoves[indexOfFrom];
        tempOffets = tempOffets & (~friendlyBitboard);
        tempOffets.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, King, flagsToEnable);
        });
    });
    
    // Castling Moves -doesn't check for friendly pieces
    if(playerColor == HUMAN_PLAYER){
        if((whiteFlags & KingSideCastle) == 0){
            validMoves.emplace_back(4, 6, King, flagsToEnable);
        }
        if((whiteFlags & QueenSideCastle) == 0){
            validMoves.emplace_back(4, 2, King, flagsToEnable);
        }
    }

    if(playerColor == AI_PLAYER){
        if((blackFlags & KingSideCastle) == 0){
            validMoves.emplace_back(60, 62, King, flagsToEnable);
        }
        if((blackFlags & QueenSideCastle) == 0){
            validMoves.emplace_back(60, 58, King, flagsToEnable);
        }
    }
}

void Chess::generatePawnMoves(BitboardElement pawnPositionBitboard, BitboardElement EnemyPositionBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves, bool white){
    BitboardElement notAFile(0xFEFEFEFEFEFEFEFEULL);
    BitboardElement notHFile(0x7F7F7F7F7F7F7F7FULL);
    BitboardElement rank3(0x0000000000FF0000ULL);
    BitboardElement rank6(0x0000FF0000000000ULL);

    BitboardElement singlePush = 0;
    BitboardElement doublePush = 0;
    BitboardElement attacksLeft = 0;
    BitboardElement attacksRight = 0;
    int singleFrom;
    int doubleFrom;
    int leftFrom;
    int rightFrom;

    if(white){
        // White moves
        singlePush =(pawnPositionBitboard << 8) & (~occupancyBitboard);
        doublePush = ((singlePush & rank3) << 8) & (~occupancyBitboard);
        attacksLeft =(pawnPositionBitboard << 7) & notHFile & EnemyPositionBitboard;
        attacksRight =(pawnPositionBitboard << 9) & notAFile & EnemyPositionBitboard;
            
        singleFrom = -8;
        doubleFrom = -16;
        leftFrom = -7;
        rightFrom = -9;
    }else{
        // Black moves
        singlePush =(pawnPositionBitboard >> 8) & (~occupancyBitboard);
        doublePush = ((singlePush & rank6) >> 8) & (~occupancyBitboard);
        attacksLeft =(pawnPositionBitboard >> 9) & notHFile & EnemyPositionBitboard;
        attacksRight =(pawnPositionBitboard >> 7) &  notAFile & EnemyPositionBitboard;

        singleFrom = 8;
        doubleFrom = 16;
        leftFrom = 9;
        rightFrom = 7;
    }

    singlePush.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + singleFrom, indexOfTo, Pawn);
    });
    doublePush.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + doubleFrom, indexOfTo, Pawn);
    });
    attacksLeft.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + leftFrom, indexOfTo, Pawn);
    });
    attacksRight.forEachBit([&](int indexOfTo){
        validMoves.emplace_back(indexOfTo + rightFrom, indexOfTo, Pawn);
    });   
}

void Chess::generateRookMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement rookAttacks = getRookAttacks(indexOfFrom, occupancyBitboard.getData());
        rookAttacks = rookAttacks & (~friendlyBitboard);
        rookAttacks.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Rook);
        });
    });
}

void Chess::generateBishopMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement bishopAttacks = getBishopAttacks(indexOfFrom, occupancyBitboard.getData());
        bishopAttacks = bishopAttacks & (~friendlyBitboard);
        bishopAttacks.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Bishop);
        });
    });
}

void Chess::generateQueenMoves(BitboardElement positionBitboard, BitboardElement friendlyBitboard, BitboardElement occupancyBitboard, std::vector<BitMove> &validMoves){
    positionBitboard.forEachBit([&](int indexOfFrom){
        BitboardElement queenAttacks = getQueenAttacks(indexOfFrom, occupancyBitboard.getData());
        queenAttacks = queenAttacks & (~friendlyBitboard);
        queenAttacks.forEachBit([&](int indexOfTo){
            validMoves.emplace_back(indexOfFrom, indexOfTo, Queen);
        });
    });    
}

//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
    _lastAIMove = BitMove();  // Reset last AI move (add this at the top of updateAI()
    int initialA = -10000000;
    int initialB = 10000000;
    int bestVal = -1000000;
    BitMove bestMove;
    bool foundBestMove = false;
    std::string state = stateString();
    allValidMoves.clear();
    findValidMoves(allValidMoves, _currentPlayer * -1, state);

    for(BitMove move: allValidMoves){

        // Perform move
        char boardSave = state[move.to];
        char pieceMoving = state[move.from];
        state[move.to] = pieceMoving;
        state[move.from] = '0';

        int moveVal = -negamax(state, 0, initialA, initialB, _currentPlayer * -1, allValidMoves.size());

        // Revert move
        state[move.from] = pieceMoving;
        state[move.to] = boardSave;

        // If the value of the current move is more than the best value, update best
        if (moveVal > bestVal) {
            foundBestMove = true;
            bestMove = move; 
            bestVal = moveVal;
        }
    }
    
    // Finally make the move
    if(foundBestMove) {
        _lastAIMove = bestMove;
        Bit* piece =_grid->getSquareByIndex(bestMove.from)->bit(); // Dupe line 
        makeMove(bestMove);
        bitMovedFromTo(*piece, *_grid->getSquareByIndex(bestMove.from), *_grid->getSquareByIndex(bestMove.to));
    }
    std::cout << "Negamax Calls: " << negaCount << std::endl;
    negaCount = 0;
}

void Chess::makeMove(BitMove &move){
    Bit* piece =_grid->getSquareByIndex(move.from)->bit(); 
    piece->setPosition(_grid->getSquareByIndex(move.to)->getPosition());
    _grid->getSquareByIndex(move.to)->setBit(piece);
    _grid->getSquareByIndex(move.from)->setBit(nullptr);
}

int Chess::evaluateBoard(const std::string& state, int moveCount){
    //AI wants high, human wants low
    int totalSum = 0;
    for(int i=0; i < state.size(); i++){
        char c = state[i];
        totalSum += combinedScore[c][i];
    }

    // Add mobility
    totalSum += moveCount * 1; // Mobility Multiplier 

    return totalSum;
}

int Chess::negamax(std::string& state, int depth, int a, int b, int playerColor, int moveCount)
{
    negaCount++;
    
    if(depth == 4){ //-------------------------------------Depth-------------------------------------
        return evaluateBoard(state, moveCount) * playerColor;
    } 
        
    std::vector<BitMove> newMoves;
    findValidMoves(newMoves, -playerColor, state);
 
    int bestVal = -10000000;
    
    
    for(BitMove move: newMoves){

        // Perform move
        char boardSave = state[move.to];
        char pieceMoving = state[move.from];
        state[move.to] = pieceMoving;
        state[move.from] = '0';
        
        bestVal = std::max(bestVal, -negamax(state, depth+1, -b, -a, -playerColor, newMoves.size()));

        // Revert move
        state[move.from] = pieceMoving;
        state[move.to] = boardSave;

        a = std::max(a, bestVal);
        if(a >= b){
            break;
        }
    }
    return bestVal;
}

// Tournament support: Set board from FEN and reinitialize game state for AI
void Chess::setBoardFromFEN(const std::string& fen) {
    // Parse FEN string - can be full FEN or just piece placement
    std::string piecePlacement = fen;
    std::string activeColor = "w";
    std::string castling = "KQkq";
    std::string enPassant = "-";

    // Check if this is a full FEN string (has spaces)
    size_t spacePos = fen.find(' ');
    if (spacePos != std::string::npos) {
        // Parse full FEN
        std::istringstream fenStream(fen);
        fenStream >> piecePlacement >> activeColor >> castling >> enPassant;
    }

    // Set visual board from piece placement
    for(int y=0; y<8; y++) {
        std::cout << ("%s", stateString().substr(y*8,8).c_str());
    }
    FENtoBoard(piecePlacement);
    for(int y=0; y<8; y++) {
        std::cout << ("%s", stateString().substr(y*8,8).c_str());
    }

    // Determine current player from FEN
    _currentPlayer = (activeColor == "w" || activeColor == "W") ? WHITE : BLACK;

    // Reinitialize game state so AI sees correct board
    _gamestate.init(stateString().c_str(), _currentPlayer);

    // TODO: Parse castling rights and en passant from FEN for more accurate state
    // For now, the basic state is sufficient for AI to calculate moves

    // Generate legal moves for the new position
    allValidMoves.clear();
    findValidMoves(allValidMoves, _currentPlayer, stateString());

    std::cout << "[Tournament] Board set from FEN. Player: "
              << (_currentPlayer == WHITE ? "White" : "Black")
              << ", Legal moves: " << allValidMoves.size() << std::endl;
}

// Tournament support: Generate FEN string from current board
std::string Chess::getFEN() const {
    std::string fen;
    fen.reserve(90);

    // Piece placement (from rank 8 to rank 1)
    for (int rank = 7; rank >= 0; --rank) {
        int emptyCount = 0;
        for (int file = 0; file < 8; ++file) {
            char piece = pieceNotation(file, rank);
            if (piece == '0') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                fen += piece;
            }
        }
        if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
        }
        if (rank > 0) {
            fen += '/';
        }
    }

    // Active color
    fen += ' ';
    fen += (_currentPlayer == WHITE) ? 'w' : 'b';

    // Castling availability (simplified - always report based on piece positions)
    fen += ' ';
    std::string castling;

    // Check if white can castle (king on e1, rooks on a1/h1)
    char e1 = pieceNotation(4, 0);
    char a1 = pieceNotation(0, 0);
    char h1 = pieceNotation(7, 0);
    if (e1 == 'K') {
        if (h1 == 'R') castling += 'K';
        if (a1 == 'R') castling += 'Q';
    }

    // Check if black can castle (king on e8, rooks on a8/h8)
    char e8 = pieceNotation(4, 7);
    char a8 = pieceNotation(0, 7);
    char h8 = pieceNotation(7, 7);
    if (e8 == 'k') {
        if (h8 == 'r') castling += 'k';
        if (a8 == 'r') castling += 'q';
    }

    fen += castling.empty() ? "-" : castling;

    // En passant target square (simplified - report as '-')
    fen += " -";

    // Halfmove clock (simplified)
    fen += " 0";

    // Fullmove number (simplified)
    fen += " 1";

    return fen;
}