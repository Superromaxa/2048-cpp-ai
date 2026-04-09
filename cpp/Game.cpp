#include "Game.h"
#include <iostream>


bool Game::is_over() const {
    return board.is_game_over();
}

bool Game::try_parse_command(char c, Direction& d) {
    switch (c) {
        case 'w': d = Direction::Up; return true;
        case 'a': d = Direction::Left; return true;
        case 's': d = Direction::Down; return true;
        case 'd': d = Direction::Right; return true;
        default: return false;
    }
}

void Game::start() {
    board = Board();
    count_steps = 0;
    board.start();
}

bool Game::make_move(Direction d) {
    bool upd = board.move(d);
    if (upd) ++count_steps;
    return upd;
}

Board& Game::get_board() {
    return board;
}

const Board& Game::get_board() const {
    return board;
}

void Game::print() const {
    std::cout << board;
}

void Game::play() {
    start();
    print();

    char command;
    while (!is_over()) {
        std::cin >> command;

        Direction dir;
        if (!try_parse_command(command, dir)) {
            std::cout << "Unknown command\n";
            continue;
        }

        if (make_move(dir)) {
            print();
        } else {
            std::cout << "Move does not change board\n";
        }
    }

    print();
    std::cout << "Game over!\n";
}

void Game::play_with_agent(Agent& agent, bool visual) {
    start();
    if (visual) print();

    while (!is_over()) {
        Direction d = agent.choose_move(board);
        make_move(d);
        if (visual) print();
    }

    if (visual) std::cout << "Game over!\n";
}

void Game::play_with_agent_and_collect(Agent& agent, Collector& collector, int game_id) {
    start();
    collector.start_game();
    int record_step = game_id % 2;
    while (!is_over()) {
        if ((count_steps + record_step) % 2) {
            collector.record_state(game_id, board, count_steps);
        }
        Direction d = agent.choose_move(board);
        make_move(d);
    }

    collector.finish_game(board);
}