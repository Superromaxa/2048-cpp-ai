#include "Agent.h"

Direction RandomAgent::choose_move(const Board& board) {
    auto moves = board.get_possible_moves();
    if (moves.empty()) {
        throw std::runtime_error("No possible moves");
    }

    std::uniform_int_distribution<> dist(0, moves.size() - 1);
    return moves[dist(gen)];
}

Direction HeuristicAgent::choose_move(const Board& board) {
    auto moves = board.get_possible_moves();
    if (moves.empty()) {
        throw std::runtime_error("No possible moves");
    }

    Direction best_move = moves[0];
    double best_value = -1e18;

    for (Direction d : moves) {
        Board copy = board;
        copy.apply_move_no_random(d);
        double cur_value = evaluator.evaluate_board(copy);

        if (cur_value > best_value) {
            best_value = cur_value;
            best_move = d;
        }
    }

    return best_move;
}

Direction ExpectimaxAgent::choose_move(const Board& board) {
    cache.clear();
    auto moves = board.get_possible_moves();
    if (moves.empty()) {
        throw std::runtime_error("No possible moves");
    }

    double best_value = -1e18;
    Direction best_dir = moves[0];

    for (Direction d : moves) {
        Board copy = board;
        copy.apply_move_no_random(d);
        double cur_value = chance_value(copy, max_depth - 1);

        if (cur_value > best_value) {
            best_value = cur_value;
            best_dir = d;
        }
    }

    return best_dir;
}

double HeuristicAgent::evaluate(const Board& board) const {
    return evaluator.evaluate_board(board);
}

std::string ExpectimaxAgent::make_key(const Board& board, int depth, bool is_chance) const {
    std::string key;
    key.reserve(96);

    for (int i = 0; i < 16; ++i) {
        key += std::to_string(board.get_el(i / 4, i % 4));
        key += ',';
    }

    key += '|';
    key += std::to_string(board.get_score());

    key += '|';
    key += std::to_string(board.get_step());

    key += '|';
    key += std::to_string(depth);

    key += '|';
    key += (is_chance ? 'C' : 'M');

    return key;
}


double ExpectimaxAgent::evaluate(const Board& board) const {
    return evaluator.evaluate_board(board);
}

double ExpectimaxAgent::max_value(const Board& board, int depth) const {
    std::string key = make_key(board, depth, false);
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    std::vector<Direction> moves = board.get_possible_moves();
    if (moves.empty()) {
        double val = evaluate(board);
        cache[key] = val;
        return val;
    }
    double max_val = -1e18;
    for (Direction d : moves) {
        Board copy = board;
        copy.apply_move_no_random(d);
        max_val = std::max(max_val, chance_value(copy, depth));
    }
    cache[key] = max_val;
    return max_val;
}

double ExpectimaxAgent::chance_value(const Board& board, int depth) const {
    std::string key = make_key(board, depth, true);
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    std::vector<int> empty_cells = board.get_empty_cells();
    int n = empty_cells.size();
    if (depth == 0 || board.is_game_over() || n == 0) {
        double val = evaluator.evaluate_board(board);
        cache[key] = val;
        return val;
    }
    double ch_val = 0;
    for (int i : empty_cells) {
        Board copy2 = board;
        copy2.place_tile(i, 2);
        double eval2 = max_value(copy2, depth - 1);

        /*Board copy4 = board;
        copy4.place_tile(i, 4);
        double eval4 = max_value(copy4, depth - 1);*/

        ch_val += (1.0 * eval2) / n;

    }

    cache[key] = ch_val;
    return ch_val;
}