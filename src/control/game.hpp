#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include "setup/team.hpp"
#include "random.hpp"
#include <memory>

struct GameSummary {
    unsigned short int whoWon;
};

class Game {
public:
    State state;
    std::array<const Team*, 2> teams;
    std::array<Agent*, 2> agents;
    Logger logger;

    Game(std::array<const Team*, 2> teams, std::array<Agent*, 2> agents, GeneratorSeed generator=getRandom());

    GameSummary play();
};
