#ifndef GAME2048_GAME_H
#define GAME2048_GAME_H
#include "Board.h"
#include "Agent.h"
#include "Collector.h"


class Game {
private:
    Board board;

    bool is_over() const;
    static bool try_parse_command(char c, Direction& d);

public:
    void start();
    bool make_move(Direction d);
    Board& get_board();
    const Board& get_board() const;
    void print() const;
    void play();
    void play_with_agent(Agent& agent, bool visual = false);
    void play_with_agent_and_collect(Agent& agent, Collector& collector, int game_id);
};

#endif //GAME2048_GAME_H