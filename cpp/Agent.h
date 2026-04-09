#ifndef GAME2048_AGENT_H
#define GAME2048_AGENT_H
#include "Evaluators.h"
#include <unordered_map>
#include <string>

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
    Evaluator& evaluator;
    double evaluate(const Board& board) const;
public:
    HeuristicAgent(Evaluator& evaluator): evaluator(evaluator) {}
    Direction choose_move(const Board& board) override;
};

class ExpectimaxAgent : public Agent {
public:
    ExpectimaxAgent(Evaluator& evaluator): evaluator(evaluator) {}
    Direction choose_move(const Board& board) override;

private:
    int max_depth = 3; // Agent move layers left
    mutable std::unordered_map<std::string, double> cache;
    Evaluator& evaluator;

    std::string make_key(const Board& board, int depth, bool is_chance) const;
    double evaluate(const Board& board) const;
    double max_value(const Board& board, int depth) const;
    double chance_value(const Board& board, int depth) const;
};

#endif //GAME2048_AGENT_H