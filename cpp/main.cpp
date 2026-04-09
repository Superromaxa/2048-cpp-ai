#include <iostream>
#include "Game.h"
#include "Agent.h"
#include "Evaluators.h"

double run_games(Agent& agent, int n_games) {
    Game game;
    long long total_score = 0;

    for (int i = 0; i < n_games; ++i) {
        game.play_with_agent(agent, false);
        total_score += game.get_board().get_score();
    }

    return static_cast<double>(total_score) / n_games;
}

int main() {
    // ExpectimaxAgent agent;
    // DataGenerator dg;
    // dg.generate_dataset(agent, 1'000, 9'000, "states.csv");

    const int n_games = 1000;
    const std::string model_path = "../artifacts/ridge_model.json";

    HeuristicEvaluator heuristic_evaluator;
    LinearRegressionEvaluator linear_evaluator(model_path);

    ExpectimaxAgent heuristic_agent(heuristic_evaluator);
    ExpectimaxAgent linear_agent(linear_evaluator);

    double heuristic_avg = run_games(heuristic_agent, n_games);
    double linear_avg = run_games(linear_agent, n_games);

    std::cout << "Heuristic Expectimax average score over " << n_games
              << " games: " << heuristic_avg << '\n';

    std::cout << "Linear Expectimax average score over " << n_games
              << " games: " << linear_avg << '\n';

    return 0;
}
