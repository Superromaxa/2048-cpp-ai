#include "Collector.h"
#include <fstream>
#include <string>

void Collector::start_game() {
    samples.clear();
}
void Collector::record_state(int game_id, const Board& board, int step) {
    Sample s;
    s.game_id = game_id;
    s.board = board.get_cells();
    s.score_at_state = board.get_score();
    s.step = step;
    int max_pos = board.get_max_position();
    s.max_tile_in_corner = max_pos == 0 || max_pos == 3 || max_pos == 12 || max_pos == 15;
    s.target = 0;
    samples.push_back(s);
}
void Collector::finish_game(const Board& board) {
    int final_score = board.get_score();
    for (Sample& s : samples) {
        s.target = final_score - s.score_at_state;
    }
}
void Collector::save_to_csv(const std::string& filename) const {
    std::ofstream out(filename, std::ios::app);
    if (out.is_open()) {
        for (const Sample& s: samples) {
            out << s.game_id << ',';
            for (int i = 0; i < 16; ++i) {
                out << s.board[i] << ',';
            }
            out << s.score_at_state << ',' << s.step << ',' << static_cast<int>(s.max_tile_in_corner) << ',' << s.target << '\n';
        }
    } else {
        throw std::runtime_error("Wrong file address");
    }
}

void Collector::write_header(const std::string& filename) {
    std::ofstream out(filename, std::ios::app);
    if (!out.is_open()) {
        throw std::runtime_error("Wrong file address");
    }

    out << "game_id,";
    for (int i = 0; i < 16; ++i) {
        out << "c" << i << ',';
    }
    out << "score,step,max_tile_in_corner,target\n";
}