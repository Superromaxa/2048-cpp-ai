#include "DataGenerator.h"
#include <filesystem>
#include <iostream>

void DataGenerator::generate_dataset(Agent& agent, int n_games, int game_id_start_index, const std::string& filename) {
    Game game;
    Collector collector;

    bool need_header = !std::filesystem::exists(filename)
                       || std::filesystem::file_size(filename) == 0;

    if (need_header) {
        collector.write_header(filename);
    }

    for (int i = 0; i < n_games; ++i) {
        game.play_with_agent_and_collect(agent, collector, game_id_start_index + i);
        collector.save_to_csv(filename);
    }
}