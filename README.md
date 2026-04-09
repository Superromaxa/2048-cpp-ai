# 2048 C++ AI

C++ implementation of the 2048 game with AI agents.

## Features
- Clean OOP architecture (Board, Game, Agent)
- Heuristic agent
- Expectimax agent with caching
- Performance optimizations

## Structure
- `Board` — game mechanics
- `Game` — game loop
- `Agent` — interface
- `HeuristicAgent`, `ExpectimaxAgent`

## Future work
- Machine Learning evaluation function
- Data collection and training pipeline

## How to run
```bash
mkdir build
cd build
cmake ..
make
./2048
