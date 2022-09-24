#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include "random.hpp"
#include <memory>

class ServerRoom;

class Game {
public:
    State state;
    std::array<Team, 2> teams;
    std::array<std::unique_ptr<Agent>, 2> agents;
    Game(std::array<Team, 2>&& teams, std::array<std::unique_ptr<Agent>, 2>&& agents, Generator generator=getRandom());

    bool play(ServerRoom* room = nullptr, bool logToTerminal = false);
};
