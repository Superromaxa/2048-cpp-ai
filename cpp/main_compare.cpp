#include <iostream>
#include <filesystem>
#include "Game.h"
#include "Agent.h"
#include "Evaluators.h"

std::string get_project_root() {
    namespace fs = std::filesystem;
    fs::path p = fs::current_path();

    // поднимаемся вверх пока не найдём папку artifacts
    while (!fs::exists(p / "artifacts")) {
        if (p == p.root_path()) {
            throw std::runtime_error("Project root not found");
        }
        p = p.parent_path();
    }

    return p.string();
}

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
    const int n_games = 1000;

    std::string root = get_project_root();
    std::string model_path = root + "/artifacts/ridge_model.json";

    HeuristicEvaluator heuristic_evaluator;
    LinearRegressionEvaluator linear_evaluator(model_path);

    ExpectimaxAgent heuristic_agent(heuristic_evaluator);
    ExpectimaxAgent linear_agent(linear_evaluator);

    double heuristic_avg = run_games(heuristic_agent, n_games);

    std::cout << "Heuristic Expectimax average score over " << n_games
              << " games: " << heuristic_avg << '\n';

    double linear_avg = run_games(linear_agent, n_games);

    std::cout << "Linear Expectimax average score over " << n_games
              << " games: " << linear_avg << '\n';

    return 0;
}
