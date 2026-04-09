#include <iostream>
#include "DataGenerator.h"
#include "Agent.h"

int main() {
    ExpectimaxAgent agent;
    DataGenerator dg;

    dg.generate_dataset(agent, 1'000, 9'000, "states.csv");

    return 0;
}