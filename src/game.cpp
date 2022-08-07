#include "state.hpp"
#include "logger.hpp"

class Game {
public:
    State state;
    std::array<Team, 2> teams;
    std::array<Agent*, 2> agents;
    Game(const std::array<Team, 2>& teams, const std::array<Agent*, 2>& agents): state(State::createStart(teams)), teams(teams), agents(agents) {};
    ~Game(){ for(Agent* agent : agents) delete agent; }

    bool play(bool isLiveServer = false, bool logToTerminal = false) {
        auto* logger = new Logger(teams);
        if(isLiveServer)
            logger = logger->liveServer();
        if(logToTerminal)
            logger = logger->logToTerminal();

        while(not state.isFinished()) {
            if(state.timestep == Timestep::BEGIN)
                agents[state.iActive]->onBegin(state);
            auto [ newState, step ] = state.advance(*agents[state.iActive]);
            state = newState;
            logger->addStep(std::move(step));
        }
        return true; //TODO return winner
    }
};
