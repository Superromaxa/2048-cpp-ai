#include <iostream>
#include "DataGenerator.h"
#include "Agent.h"
#include "Evaluators.h"

int main() {
    HeuristicEvaluator heuristic_evaluator;

    RandomAgent ragent;
    HeuristicAgent hagent(heuristic_evaluator);
    ExpectimaxAgent exagent(heuristic_evaluator);

    DataGenerator dg;

    dg.generate_dataset(ragent, 1000, 0, "states.csv");
    std::cout << "random agent finished playing\n";

    dg.generate_dataset(hagent, 3000, 1000, "states.csv");
    std::cout << "heuristic agent finished playing\n";

    dg.generate_dataset(exagent, 6000, 4000, "states.csv");
    std::cout << "expectimax agent finished playing\n";

    return 0;
}