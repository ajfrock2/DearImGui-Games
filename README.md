# ImGui Games
– Austin Frock

## OVERVIEW
This repository contains a collection of games built in C++ using Dear ImGui, all based around a bitboard and bit-holder system for managing game state.

These games were created during a game development class, where the focus was on writing efficient, performance-oriented code and exploring different ways to structure game logic. Using bitboards made performance a real concern, which pushed me to think carefully about how data was stored, accessed, and updated.

A major part of these projects involved designing AI behavior for games. Over time, I experimented with multiple AI strategies and approaches, learning what worked well in practice and what didn’t. Along the way, I also picked up a lot of C++ techniques aimed at optimizing code, which became especially important given the real-time nature of the games.

### Tic Tac Toe

This was the first game I created in ImGui and served as my introduction to building game AI.

The main focus of this game was understanding terminal states and how they affect decision-making in a game. Working through these concepts gave me a solid foundation for the AI techniques I would use in later games, and helped me understand why clearly defining game-ending conditions is so important when designing AI logic.

### Connect 4

This game built on the ideas introduced in Tic Tac Toe and forced me to learn Negamax due to the increased complexity of the game.

Connect 4 requires much deeper lookahead and more careful evaluation of future moves, which made simpler AI approaches impractical. This game helped me understand how game AI works at its core by evaluating board states and scoring positions based on possible outcomes. Implementing Negamax helped me manage this complexity and marked a clear step forward in the AI techniques I was learning.

### Catching Strays

I built this game over a single weekend for a small game jam. Unlike my earlier two-player turn-based projects, this game forced me to dig deeper into my own code to really understand how it worked.  

By doing so, I was able to manipulate the game’s logic to behave differently from a traditional turn-based system. This experience helped me gain confidence in reading and adjusting my code on the fly, and reinforced the importance of structuring code in a way that makes it flexible and adaptable.

### Chess

Chess was the project where I really mastered my understanding of how game AI works. The complexity of the game required me to apply everything I had learned in earlier games and bring it all together.

This project also pushed me to explore a variety of C++ techniques to optimize my code for performance. Writing efficient data structures, managing memory carefully, and streamlining algorithms became essential to making the AI responsive and effective given the complexity of Chess.

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