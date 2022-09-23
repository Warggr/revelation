#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include "random.hpp"

class ServerRoom;

class Game {
public:
    State state;
    std::array<Team, 2> teams;
    std::array<Agent*, 2> agents;
    Game(std::array<Team, 2>&& teams, const std::array<Agent*, 2>& agents, Generator generator=getRandom());

    bool play(ServerRoom* room = nullptr, bool logToTerminal = false);
};
