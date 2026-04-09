#include "Board.h"
#include <iostream>
#include <iomanip>

Board::Board(): gen(std::random_device{}()), tile_dist(1, 10) {}

bool Board::possible_move(Direction d) const {
    Board bd = *this;
    bool res = bd.apply_move(d);
    return res;
}

bool Board::move_line(int i) {
    bool flag = false;
    // step 1
    for (int z = 0; z < 4; ++z) {
        if (board[i * 4 + z] == 0) {
            int j = z;
            while (j < 4 && board[i * 4 + j] == 0) {
                ++j;
            }
            if (j < 4) {
                board[i * 4 + z] = board[i * 4 + j];
                board[i * 4 + j] = 0;
                flag = true;
            }
        }
    }

    // step 2
    for (int l = 0; l < 3; ++l) {
        if (board[i * 4 + l] != 0 && board[i * 4 + l] == board[i * 4 + l + 1]) {
            board[i * 4 + l] *= 2;
            score += board[i * 4 + l];
            board[i * 4 + l + 1] = 0;
            flag = true;
            ++l;
        }
    }

    // step 3
    for (int z = 0; z < 4; ++z) {
        if (board[i * 4 + z] == 0) {
            int j = z;
            while (j < 4 && board[i * 4 + j] == 0) {
                ++j;
            }
            if (j < 4) {
                board[i * 4 + z] = board[i * 4 + j];
                board[i * 4 + j] = 0;
                flag = true;
            }
        }
    }
    return flag;
}

void Board::transpose() {
    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            std::swap(board[i * 4 + j], board[j * 4 + i]);
        }
    }
}

void Board::reverse_line(int i) {
    std::swap(board[i * 4], board[i * 4 + 3]);
    std::swap(board[i * 4 + 1], board[i * 4 + 2]);
}

void Board::add_number() {
    int number = tile_dist(gen);
    int val = (number > 8 ? 4 : 2);
    std::vector<int> zeros;
    for (int i = 0; i < 16; ++i) {
        if (board[i] == 0) zeros.push_back(i);
    }
    if (zeros.empty()) return;

    std::uniform_int_distribution<> dist(0, zeros.size() - 1);
    int random_index = dist(gen);
    int cell = zeros[random_index];
    board[cell] = val;
}

bool Board::apply_move(Direction d) {
    bool res = false;
    bool moved = false;
    switch (d) {
        case Direction::Up:
            transpose();
            for (int i = 0; i < 4; ++i) {
                moved = move_line(i);
                res = res || moved;
            }
            transpose();
            return res;
        case Direction::Down:
            transpose();
            for (int i = 0; i < 4; ++i) {
                reverse_line(i);
                moved = move_line(i);
                res = res || moved;
                reverse_line(i);
            }
            transpose();
            return res;
        case Direction::Left:
            for (int i = 0; i < 4; ++i) {
                moved = move_line(i);
                res = res || moved;
            }
            return res;
        case Direction::Right:
            for (int i = 0; i < 4; ++i) {
                reverse_line(i);
                moved = move_line(i);
                res = res || moved;
                reverse_line(i);
            }
            return res;
        default:
            return res;
    }
}

bool Board::is_game_over() const {
    for (Direction d : dirs) {
        if (possible_move(d)) {
            return false;
        }
    }
    return true;
}

int Board::get_el(size_t a, size_t b) const {
    if (a < 4 && b < 4) {
        return board[a * 4 + b];
    }
    return -1;
}

void Board::set_el(int e, size_t a, size_t b) {
    if (a < 4 && b < 4) {
        board[a * 4 + b] = e;
    }
}

void Board::set_el(int e, size_t a) {
    if (a < 16) {
        board[a] = e;
    }
}

int Board::get_score() const {
    return score;
}

int Board::get_max_position() const {
    int max = 0;
    int pos = -1;
    for (int i = 0; i < 16; ++i) {
        if (board[i] > max) {
            max = board[i];
            pos = i;
        }
    }
    return pos;
}

bool Board::move(Direction d) {
    bool upd = apply_move(d);
    if (upd) add_number();
    return upd;
}

void Board::start() {
    add_number();
    add_number();
}

bool Board::apply_move_no_random(Direction d) {
    bool upd = apply_move(d);
    return upd;
}

void Board::place_tile(int index, int value) {
    if (0 <= index && index < 16 && board[index] == 0 && (value == 2 || value == 4)) {
        board[index] = value;
    }
}

std::vector<Direction> Board::get_possible_moves() const {
    std::vector<Direction> pos_moves;
    for (Direction d : dirs) {
        if (possible_move(d)) {
            pos_moves.emplace_back(d);
        }
    }
    return pos_moves;
}

std::vector<int> Board::get_empty_cells() const {
    std::vector<int> cells;
    for (int i = 0; i < 16; ++i) {
        if (board[i] == 0) {
            cells.push_back(i);
        }
    }
    return cells;
}

int Board::get_empty_count() const {
    int empty = 0;
    for (int i = 0; i < 16; ++i) {
        if (board[i] == 0) {
            ++empty;
        }
    }
    return empty;
}

std::array<int, 16> Board::get_cells() const {
    return board;
}

std::ostream& operator<<(std::ostream& os, const Board& b) {
    int tmp = 0;
    for (int i = 0; i < 4; ++i) {
        os << "+-------+-------+-------+-------+\n";
        for (int j = 0; j < 4; ++j) {
            os << "|" << std::setw(7);
            tmp = b.get_el(i, j);
            if (tmp != 0) {
                os << tmp;
            } else {
                os << '.';
            }
        }
        os << '|' << '\n';
    }
    os << "+-------+-------+-------+-------+\n";
    os << "score: " << b.score << '\n';

    return os;
}