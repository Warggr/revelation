#include "state.hpp"
#include "logger.hpp"
#include "game.hpp"

Game::Game(std::array<Team, 2>&& teams, const std::array<Agent*, 2>& agents):
    state(State::createStart(teams)), teams(std::move(teams)), agents(agents)
{
};

bool Game::play(bool isLiveServer, bool logToTerminal) {
    auto* logger = new Logger(teams);
    if(isLiveServer)
        logger = logger->liveServer();
    if(logToTerminal)
        logger = logger->logToTerminal();

    while(not state.isFinished()) {
        if(state.timestep == Timestep::BEGIN)
            agents[state.iActive]->onBegin(state);
        auto [ newState, step ] = state.advance(*agents[state.iActive], *agents[1 - state.iActive]);
        state = newState;
        logger->addStep(std::move(step));
    }
    return true; //TODO return winner
}
