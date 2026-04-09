# 2048 AI with Expectimax and Machine Learning

## Overview

This project implements the game **2048** in C++ with a modular architecture and extends it with artificial intelligence agents and a machine learning pipeline for learning a board evaluation function.

The system consists of:
- a **C++ game engine** with multiple AI agents,
- a **data generation pipeline** for collecting gameplay states,
- a **Python-based ML pipeline** for training models,
- and an integration path for replacing heuristic evaluation with a learned model.

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

## Experimental Results

Best models ranked by validation RMSE:
```
hgb_extended_raw:     valid_rmse=5935.921, test_rmse=5828.937, spearman=0.5436
ridge_extended_raw:   valid_rmse=6178.011, test_rmse=6093.105, spearman=0.4712
ridge_base_raw:       valid_rmse=6308.182, test_rmse=6237.769, spearman=0.4026
hgb_extended_log1p:   valid_rmse=6428.944, test_rmse=6300.286, spearman=0.5340
ridge_extended_log1p: valid_rmse=6802.227, test_rmse=6663.695, spearman=0.4600
```

### Observations

- Gradient boosting significantly outperforms linear models  
- Extended feature set improves performance across all models  
- Spearman correlation indicates moderate ranking quality of predicted states  

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

## Summary

This project implements a full pipeline:

- game simulation in C++  
- data generation from gameplay  
- supervised learning of value functions  
- evaluation and model comparison  

The approach combines classical search (Expectimax) with data-driven evaluation, providing a foundation for improving gameplay performance.
