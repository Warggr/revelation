#include "state.hpp"
#include "logger.hpp"

class Game {
public:
    State state;
    std::array<Team, 2> teams;
    std::array<Agent*, 2> agents;
    Game(const std::array<Team, 2>& teams, const std::array<Agent*, 2>& agents);
    ~Game();

    bool play(bool isLiveServer = false, bool logToTerminal = false);
};
