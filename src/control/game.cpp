#include "game.hpp"
#include "agent.hpp"
#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include <random>

Generator getRandom(){
    std::random_device rd;
    return Generator(rd());
}

Game::Game(std::array<const Team*, 2> teams, std::array<std::unique_ptr<Agent>, 2>&& agents, Generator generator):
    state(State::createStart(teams, generator)), teams(teams), agents(std::move(agents))
{
}

Game Game::createFromAgents(std::array<std::unique_ptr<Agent>, 2>&& agents, const UnitsRepository& repo) {
    std::array<const Team*, 2> teams = { &agents[0]->getTeam(repo), &agents[1]->getTeam(repo) };
    return Game(teams, std::move(agents));
}

bool Game::play(ServerRoom* serverRoom, std::ostream* logFile) {
    auto* logger = new Logger(state, teams);
    if(serverRoom)
        logger = logger->liveServer(*serverRoom);
    if(logFile)
        logger = logger->logToFile(*logFile);
    try { //A disconnected agent should be able to throw an exception and kill the game.
        unsigned short int winner;
        while ((winner = state.getWinner()) == 0) {
            if (state.timestep == Timestep::BEGIN)
                agents[state.iActive]->onBegin(state);
            auto[newState, step] = state.advance(*agents[state.iActive], *agents[1 - state.iActive]);
            state = newState;
            logger->addStep(std::move(step));
        }
        delete logger;
        return winner;
    } catch(AgentSurrenderedException& ex) {
        delete logger;
        return 1 - ex.id;
    }
}

std::tuple<State, uptr<Step>> State::advance(Agent& active, Agent& opponent) const {
    if(!unresolvedSpecialAbility.empty()){
        State copy(*this);

        Effect* effect = copy.unresolvedSpecialAbility.front();
        copy.unresolvedSpecialAbility.pop_front();

        Agent& whoDecides = effect->opponentChooses() ? opponent : active;
        unsigned int decision = whoDecides.getSpecialAction(copy, *effect);

        uptr<Step> step = effect->resolve(copy, decision);
        return std::make_tuple<State, uptr<Step>>( std::move(copy), std::move(step) );
    }
    switch(this->timestep){
    case ACTED:
        return this->beginTurn();
    case BEGIN: {
        return this->stepDraw();
    }
    case DREW: {
        DiscardDecision decision = active.getDiscard(*this);
        return this->stepDiscard(decision);
    }
    case DISCARDED:
    case MOVEDfirst: {
        MoveDecision decision = active.getMovement(*this, (timestep==DISCARDED ? 0 : 1));
        return this->stepMov(decision);
    }
    case MOVEDlast: {
        AbilityDecision decision = active.getAbility(*this);
        return this->stepAbil(decision);
    }
    case ABILITYCHOSEN: {
        ActionDecision decision = active.getAction(*this);
        return this->stepAct(decision);
    }
    default: //shouldn't happen
        throw 1;
        //return std::make_tuple<State, uptr<Step>>({}, {});
    }
}
