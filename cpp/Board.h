#ifndef INC_2048_BOARD_H
#define INC_2048_BOARD_H
#include <array>
#include <random>
#include <vector>

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

class Board {
private:
    std::array<int, 16> board = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int score = 0;
    std::mt19937 gen;
    std::uniform_int_distribution<> tile_dist;

    static constexpr std::array<Direction, 4> dirs = {
        Direction::Up,
        Direction::Down,
        Direction::Left,
        Direction::Right
    };

    bool possible_move(Direction d) const; // checks whether move d can be made
    bool move_line(int i); // performs left slide for a line
    void transpose(); // helping method which transposes the board (to implement moves)
    void reverse_line(int i); // helping method which reverses one line of the board (to implement moves)
    void add_number(); // adds number on the board
    bool apply_move(Direction d); // applies move in direction d if possible, returns true if it was made

public:
    Board();
    bool is_game_over() const; // checks whether game is over or not
    int get_el(size_t a, size_t b) const; // returns element placed in position (a, b)
    void set_el(int e, size_t a, size_t b); // sets tile (a, b) value e
    void set_el(int e, size_t a); // sets tile a (numbered from 0 to 15) value e
    int get_score() const; // returns current score
    int get_max_position() const; // returns the index (0-15) of the max tile on the board
    friend std::ostream& operator<<(std::ostream& os, const Board& b); // prints the board
    bool move(Direction d); // applies move in direction d and updates the board randomly adding a number
    void start(); // creates an empty board with 2 tiles
    bool apply_move_no_random(Direction d); // applies move in direction d without adding new tiles
    void place_tile(int index, int value); // sets the tile index value iff it was empty
    std::vector<Direction> get_possible_moves() const; // returns directions of possible moves
    std::vector<int> get_empty_cells() const; // returns indices (0-15) of empty cells
    int get_empty_count() const; // returns the number of empty cells
    std::array<int, 16> get_cells() const; // returns all the cells
};

std::ostream& operator<<(std::ostream& os, const Board& b);

#endif //INC_2048_BOARD_H