# ImGui Games
– Austin Frock

## OVERVIEW
This repository contains a collection of games built in C++ using Dear ImGui, all based around a bitboard and bit-holder system for managing game state.

These games were created during a game development class, where the focus was on writing efficient, performance-oriented code and exploring different ways to structure game logic. Using bitboards made performance a real concern, which pushed me to think carefully about how data was stored, accessed, and updated.

A major part of these projects involved designing AI behavior for games. Over time, I experimented with multiple AI strategies and approaches, learning what worked well in practice and what didn’t. Along the way, I also picked up a lot of C++ techniques aimed at optimizing code, which became especially important given the real-time nature of the games.

## GAMES

### Tic Tac Toe
A classic 3x3 grid game where two players take turns. This version includes a basic AI that evaluates terminal states to determine optimal moves.

### Connect 4
A vertical 7x6 grid game where players drop discs to form a line of four. Includes an AI implementation using the Negamax algorithm for decision-making.

### Catching Strays
A small experimental game built for a game jam. Features modified turn-based logic with custom rules for puzzle-like gameplay.

### Chess
A full Chess implementation with AI capable of evaluating board states and making strategic moves. Optimized for performance using advanced C++ techniques.

## HOW TO RUN

These games are built in C++ using Dear ImGui and CMake. Follow these steps to build and run them:

```bash
# 1. Clone the repository
git clone https://github.com/ajfrock2/DearImGui-Games.git
cd DearImGui-Games

# 2. Create and enter a build directory
mkdir build
cd build

# 3. Generate build files with CMake
cmake ..

# 4. Build the project
cmake --build .

# 5. Run the executable

# Windows (run from build folder, usually in Debug or Release)
Debug\demo.exe
# or
Release\demo.exe

# Linux/macOS (run from build folder)
./demo