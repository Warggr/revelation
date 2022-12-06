#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include "setup/units_repository.hpp"
#include "random.hpp"
#include <memory>

class ServerRoom;

class Game {
public:
    State state;
    std::array<const Team*, 2> teams;
    std::array<std::unique_ptr<Agent>, 2> agents;
    Game(std::array<const Team*, 2> teams, std::array<std::unique_ptr<Agent>, 2>&& agents, Generator generator=getRandom());
    static Game createFromAgents(std::array<std::unique_ptr<Agent>, 2>&& agents, const UnitsRepository& repo);

    bool play(ServerRoom* room = nullptr, std::ostream* logFile = nullptr);
};
