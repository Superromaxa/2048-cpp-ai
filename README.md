# 2048 AI with Expectimax and Machine Learning

## Overview

This project implements the game **2048** in C++ with a modular architecture and extends it with artificial intelligence agents and a machine learning pipeline for learning a board evaluation function.
The project combines a search-based AI (Expectimax) with a learned evaluation function trained on large-scale gameplay data, enabling measurable improvements over handcrafted heuristics.

---

## Results

### Model performance

| Model | Valid RMSE | Test RMSE | Spearman |
|------|-----------|-----------|----------|
| HGB (extended, raw) | 5935.9 | 5828.9 | 0.5436 |
| Ridge (extended, raw) | 6178.0 | 6093.1 | 0.4712 |

### Gameplay performance

| Agent | Average Score (1000 games) |
|------|---------------------------|
| Heuristic Expectimax | 15707.8 |
| ML-based Expectimax (Linear) | 16343.4 |

The learned evaluation function improves the agent performance compared to the heuristic baseline.

---

## Key Features

- Clean C++ implementation of 2048 with separation of concerns (`Board`, `Game`)
- Multiple AI agents:
  - Random baseline
  - Heuristic-based agent
  - Expectimax agent with state caching
- Data collection during gameplay (millions of states)
- Feature engineering and model training in Python
- Evaluation using regression and ranking metrics
- Export of trained linear models for integration into C++

---

## Repository Structure
```
game2048/
├── cpp/              # C++ game engine and agents
├── python/           # ML training pipeline
├── artifacts/        # training results and exported models
├── data/             # datasets (not tracked in git)
├── README.md
└── .gitignore
```

---

## C++ Engine

The core game logic is implemented in C++ with the following components:

### Core Classes

- `Board` — handles:
  - grid state
  - moves and merges
  - random tile generation
  - game termination logic

- `Game` — controls:
  - gameplay loop
  - interaction with agents

---

### Agents

All agents implement a unified interface:

#### 1. RandomAgent
Selects a random valid move.

#### 2. HeuristicAgent
Evaluates board states using handcrafted features such as:
- number of empty cells
- merge potential
- monotonicity
- position of the maximum tile

#### 3. ExpectimaxAgent
Implements a search-based strategy:
- alternates between decision nodes (player moves) and chance nodes (random tile spawn)
- models tile probabilities (2 with 0.9, 4 with 0.1)
- uses depth-limited search
- applies memoization (state caching) to avoid recomputation

---

### Agent Performance

Average scores over 5,000 games:

| Agent       | Average Score |
|------------|--------------|
| Random     | ~1,100       |
| Heuristic  | ~3,200       |
| Expectimax | ~13,000      |

---

## Data Generation Pipeline

Gameplay data is collected using:

- `Collector` — records:
  - board state (`c0`–`c15`)
  - current score
  - step index
  - additional features (e.g. max tile in corner)
  - game identifier

- `DataGenerator` — orchestrates:
  - running multiple games with a selected agent
  - sampling states (e.g. every second state)
  - writing data to CSV

### Target Definition

For each recorded state:

target = final_score - score_at_state

This represents the expected future reward from a given state.

---

## Machine Learning Pipeline

Implemented in `python/train_models.py`.

### Workflow

1. Load dataset from CSV  
2. Validate schema  
3. Build features  
4. Split data by `game_id` (to avoid leakage)  
5. Train multiple models  
6. Evaluate on validation and test sets  
7. Export best linear model  

---

### Feature Engineering

- Log-scaled tile values: `log2(value + 1)`
- Board statistics:
  - number of empty cells
  - maximum tile
  - sum of tiles
- Game context:
  - current score
  - step index
  - max tile in corner

---

### Models

The following models are trained and compared:

- Ridge regression (baseline and extended feature sets)
- Histogram Gradient Boosting (sklearn)

Both raw and log-transformed targets are evaluated.

---

## Model Export

The best linear model is exported as:
artifacts/ridge_model.json


This enables integration into the C++ Expectimax agent as a learned evaluation function.

---

## Future Work

- Integrate trained model into Expectimax evaluation  
- Compare gameplay performance vs heuristic evaluation  
- Explore ranking-based objectives instead of regression  
- Add neural network baseline  
- Optimize inference speed in C++  

---

## Requirements

### C++
- CMake  
- C++17 or higher  

### Python
- pandas  
- numpy  
- scikit-learn  

---

## Entry Points

- `main_collect_data.cpp`  
  Generates training data by running multiple games with different agents and saving states to CSV.

- `main_compare.cpp`  
  Runs multiple games and compares different evaluation strategies (heuristic vs ML-based).

---

## How to run

### Build C++ engine

```bash
cd cpp
mkdir build && cd build
cmake ..
make```

### Collect dataset

```./collect_data```

### Train model (Python)

```cd ../../python
python train_models.py```

### Compare agents

```./compare```

### Note
The program automatically resolves project root, so it can be run from any directory.

---

## Summary

This project implements a full pipeline:

- game simulation in C++  
- data generation from gameplay  
- supervised learning of value functions  
- evaluation and model comparison  

Unlike standard implementations, this project replaces handcrafted evaluation in Expectimax with a learned value function trained from gameplay data, demonstrating improved decision quality.

