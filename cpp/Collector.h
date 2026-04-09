#ifndef GAME2048_COLLECTOR_H
#define GAME2048_COLLECTOR_H
#include "Board.h"
#include <string>

struct Sample {
    std::array<int, 16> board;
    int game_id;
    int score_at_state = 0;
    int step = 0;
    bool max_tile_in_corner = false;
    double target = 0;
};


class Collector {
private:
    std::vector<Sample> samples;

public:
    void start_game();
    void record_state(int game_id, const Board& board, int step);
    void finish_game(const Board& board);
    void save_to_csv(const std::string& filename) const;
    void write_header(const std::string& filename);
};


#endif //GAME2048_COLLECTOR_H