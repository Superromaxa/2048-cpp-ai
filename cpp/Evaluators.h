#ifndef GAME2048_EVALUATORS_H
#define GAME2048_EVALUATORS_H
#include "Board.h"
#include <string>
#include <vector>

class Evaluator {
public:
    virtual double evaluate_board(const Board& board) const = 0;
    virtual ~Evaluator() = default;
};

class HeuristicEvaluator : public Evaluator {
public:
    double count_empty_bonus(const Board& board) const;
    double count_corner_bonus(const Board& board) const;
    double count_merge_bonus(const Board& board) const;
    double count_monotonicity_bonus(const Board& board) const;
    double evaluate_board(const Board &board) const override;
};

class LinearRegressionEvaluator : public Evaluator {
public:
    explicit LinearRegressionEvaluator(const std::string& model_path);

    double evaluate_board(const Board& board) const override;

private:
    std::vector<std::string> feature_names_;
    std::vector<double> weights_;
    double bias_ = 0.0;

    static int tile_to_log2(int value);
    static int count_mergeable(const Board& board);
    static int count_monotonicity(const Board& board);
};


#endif //GAME2048_EVALUATORS_H
