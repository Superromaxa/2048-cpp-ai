#ifndef GAME2048_DATAGENERATOR_H
#define GAME2048_DATAGENERATOR_H

#include "Game.h"
#include <string>

class DataGenerator {
public:
    void generate_dataset(Agent& agent, int n_games, int game_id_start_index, const std::string& filename);
};


#endif //GAME2048_DATAGENERATOR_H