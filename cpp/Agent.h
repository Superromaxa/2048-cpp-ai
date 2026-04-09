#ifndef GAME2048_AGENT_H
#define GAME2048_AGENT_H
#include "Board.h"
#include <unordered_map>
#include <string>

class BoardEvaluator {
public:
    double count_empty_bonus(const Board& board) const;
    double count_corner_bonus(const Board& board) const;
    double count_merge_bonus(const Board& board) const;
    double count_monotonicity_bonus(const Board& board) const;
    double evaluate_board(const Board& board) const;
};

class Agent {
public:
    virtual Direction choose_move(const Board& board) = 0;
    virtual ~Agent() = default;
};

class RandomAgent : public Agent {
private:
    std::mt19937 gen;

public:
    RandomAgent() : gen(std::random_device{}()) {}
    Direction choose_move(const Board& board) override;
};

class HeuristicAgent : public Agent {
private:
    BoardEvaluator evaluator;
    double evaluate(const Board& board) const;
public:
    Direction choose_move(const Board& board) override;
};

class ExpectimaxAgent : public Agent {
public:
    Direction choose_move(const Board& board) override;

private:
    int max_depth = 3; // Agent move layers left
    BoardEvaluator evaluator;
    mutable std::unordered_map<std::string, double> cache;

    std::string make_key(const Board& board, int depth, bool is_chance) const;
    double evaluate(const Board& board) const;
    double max_value(const Board& board, int depth) const;
    double chance_value(const Board& board, int depth) const;
};

#endif //GAME2048_AGENT_H