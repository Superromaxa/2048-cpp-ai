#include "Evaluators.h"
#include "../json/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

double HeuristicEvaluator::count_empty_bonus(const Board& board) const {
    return board.get_empty_count();
}

double HeuristicEvaluator::count_corner_bonus(const Board& board) const {
    int max_pos = board.get_max_position();
    int max_corner = 0;
    if (max_pos == 0 || max_pos == 3 || max_pos == 12 || max_pos == 15) {
        max_corner = 1;
    }
    return max_corner;
}

double HeuristicEvaluator::count_merge_bonus(const Board& board) const {
    int mergeable = 0;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int cur = board.get_el(i, j);
            if (cur == 0) continue;

            if (board.get_el(i + 1, j) == cur) {
                ++mergeable;
            }
            if (board.get_el(i, j + 1) == cur) {
                ++mergeable;
            }
        }
    }

    return mergeable;
}

double HeuristicEvaluator::count_monotonicity_bonus(const Board& board) const {
    int bonus = 0;

    // rows
    for (int i = 0; i < 4; ++i) {
        int dec = 0;
        int inc = 0;

        for (int j = 0; j < 3; ++j) {
            int a = board.get_el(i, j);
            int b = board.get_el(i, j + 1);

            if (a >= b) {
                dec += a - b;
            } else {
                inc += b - a;
            }
        }

        bonus += std::max(dec, inc);
    }

    // columns
    for (int j = 0; j < 4; ++j) {
        int dec = 0;
        int inc = 0;

        for (int i = 0; i < 3; ++i) {
            int a = board.get_el(i, j);
            int b = board.get_el(i + 1, j);

            if (a >= b) {
                dec += a - b;
            } else {
                inc += b - a;
            }
        }

        bonus += std::max(dec, inc);
    }

    return bonus;
}

double HeuristicEvaluator::evaluate_board(const Board &board) const {
    double evaluation = 300 * count_empty_bonus(board) + 100 * count_corner_bonus(board) + 70 * count_merge_bonus(board) + 0.5 * count_monotonicity_bonus(board);
    return evaluation;
}

LinearRegressionEvaluator::LinearRegressionEvaluator(const std::string& model_path) {
    std::ifstream file(model_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open model file: " + model_path);
    }

    nlohmann::json model_json;
    file >> model_json;

    feature_names_ = model_json.at("feature_names").get<std::vector<std::string>>();
    weights_ = model_json.at("weights").get<std::vector<double>>();
    bias_ = model_json.at("bias").get<double>();

    if (feature_names_.size() != weights_.size()) {
        throw std::runtime_error("Invalid model: feature_names size does not match weights size");
    }
}

int LinearRegressionEvaluator::tile_to_log2(int value) {
    if (value == 0) {
        return 0;
    }

    int power = 0;
    while (value > 1) {
        value /= 2;
        ++power;
    }
    return power;
}

int LinearRegressionEvaluator::count_mergeable(const Board& board) {
    int mergeable = 0;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            int cur = board.get_el(i, j);
            if (cur == 0) {
                continue;
            }

            if (i + 1 < 4 && board.get_el(i + 1, j) == cur) {
                ++mergeable;
            }
            if (j + 1 < 4 && board.get_el(i, j + 1) == cur) {
                ++mergeable;
            }
        }
    }

    return mergeable;
}

int LinearRegressionEvaluator::count_monotonicity(const Board& board) {
    int bonus = 0;

    for (int i = 0; i < 4; ++i) {
        int dec = 0;
        int inc = 0;

        for (int j = 0; j < 3; ++j) {
            int a = board.get_el(i, j);
            int b = board.get_el(i, j + 1);

            if (a >= b) {
                dec += a - b;
            } else {
                inc += b - a;
            }
        }

        bonus += std::max(dec, inc);
    }

    for (int j = 0; j < 4; ++j) {
        int dec = 0;
        int inc = 0;

        for (int i = 0; i < 3; ++i) {
            int a = board.get_el(i, j);
            int b = board.get_el(i + 1, j);

            if (a >= b) {
                dec += a - b;
            } else {
                inc += b - a;
            }
        }

        bonus += std::max(dec, inc);
    }

    return bonus;
}

double LinearRegressionEvaluator::evaluate_board(const Board& board) const {
    const std::array<int, 16> cells = board.get_cells();

    int empty_count = 0;
    int sum_tiles = 0;
    int max_tile = 0;

    for (int value : cells) {
        sum_tiles += value;
        if (value == 0) {
            ++empty_count;
        }
        max_tile = std::max(max_tile, value);
    }

    const int max_pos = board.get_max_position();
    const int non_zero_count = 16 - empty_count;
    const int max_tile_log2 = tile_to_log2(max_tile);
    const int mergeable_count = count_mergeable(board);
    const int monotonicity = count_monotonicity(board);

    const bool max_tile_in_corner =
        (max_pos == 0 || max_pos == 3 || max_pos == 12 || max_pos == 15);

    const bool max_tile_on_edge =
        (max_pos == 0 || max_pos == 1 || max_pos == 2 || max_pos == 3 ||
         max_pos == 4 || max_pos == 7 || max_pos == 8 || max_pos == 11 ||
         max_pos == 12 || max_pos == 13 || max_pos == 14 || max_pos == 15);

    double prediction = bias_;

    for (size_t i = 0; i < feature_names_.size(); ++i) {
        const std::string& name = feature_names_[i];
        double feature_value = 0.0;

        if (name.rfind("log_c", 0) == 0) {
            int idx = std::stoi(name.substr(5));
            feature_value = static_cast<double>(tile_to_log2(cells[idx]));
        } else if (name == "score") {
            feature_value = static_cast<double>(board.get_score());
        } else if (name == "step") {
            feature_value = static_cast<double>(board.get_step());
        } else if (name == "max_tile_in_corner") {
            feature_value = max_tile_in_corner ? 1.0 : 0.0;
        } else if (name == "empty_count") {
            feature_value = static_cast<double>(empty_count);
        } else if (name == "max_tile_log2") {
            feature_value = static_cast<double>(max_tile_log2);
        } else if (name == "sum_tiles") {
            feature_value = static_cast<double>(sum_tiles);
        } else if (name == "mergeable_count") {
            feature_value = static_cast<double>(mergeable_count);
        } else if (name == "monotonicity") {
            feature_value = static_cast<double>(monotonicity);
        } else if (name == "max_tile_position") {
            feature_value = static_cast<double>(max_pos);
        } else if (name == "max_tile_on_edge") {
            feature_value = max_tile_on_edge ? 1.0 : 0.0;
        } else if (name == "non_zero_count") {
            feature_value = static_cast<double>(non_zero_count);
        } else {
            throw std::runtime_error("Unknown feature name in model: " + name);
        }

        prediction += weights_[i] * feature_value;
    }

    return std::max(0.0, prediction);
}
