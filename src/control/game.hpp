#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include "setup/team.hpp"
#include "random.hpp"
#include <memory>

struct GameSummary {
    unsigned short int whoWon;
};

constexpr unsigned int NB_AGENTS = 2;

class Game {
public:
    State state;
    std::array<const Team*, NB_AGENTS> teams;
    std::array<Agent*, NB_AGENTS> agents;
    Logger logger;

    Game(std::array<const Team*, NB_AGENTS> teams, std::array<Agent*, NB_AGENTS> agents, GeneratorSeed generator=getRandom());

    GameSummary play();
};
