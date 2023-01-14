#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include "setup/team.hpp"
#include "random.hpp"
#include "constants.hpp"
#include <memory>
#include <chrono>

struct AgentGameSummary {
    std::chrono::duration<double> total_time = std::chrono::duration<double>::zero();

    void addTime(std::chrono::duration<double> time){ total_time += time; }
};

struct GameSummary {
    static constexpr unsigned short int NOBODY_WON = 0;

    unsigned short int whoWon = NOBODY_WON;
    unsigned short int nbSteps = 0;
    std::array<AgentGameSummary, NB_AGENTS> agents;
};

class Game {
public:
    State state;
    std::array<const Team*, NB_AGENTS> teams;
    std::array<Agent*, NB_AGENTS> agents;
    Logger logger;

    Game(std::array<const Team*, NB_AGENTS> teams, std::array<Agent*, NB_AGENTS> agents, GeneratorSeed generator=getRandom());

    GameSummary play(int maxTurns = -1);
};
