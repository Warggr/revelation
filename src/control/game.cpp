#include "game.hpp"
#include "agent.hpp"
#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include <random>
#include <cassert>

GeneratorSeed getRandom(){
    return std::random_device()();
}

Game::Game(std::array<const Team*, 2> teams, std::array<Agent*, 2> agents, GeneratorSeed seed):
    state(State::createStart(teams, Generator(seed))), teams(teams), agents(agents)
    , logger(state, teams, seed)
{
    for(unsigned int i=0; i<2; i++){
        assert(agents[i]->getId() == i);
    }
}

GameSummary Game::play(int maxSteps) {
    GameSummary summary;
    try { //A disconnected agent should be able to throw an exception and kill the game.
        while ((summary.whoWon = state.getWinner()) == 0 and summary.nbSteps++ != maxSteps) {
            auto start = std::chrono::steady_clock::now();
            if (state.timestep == Timestep::BEGIN)
                agents[state.iActive]->onBegin(state);
            auto[newState, step] = state.advance(*agents[state.iActive], *agents[1 - state.iActive]);
            auto end = std::chrono::steady_clock::now();
            summary.agents[state.iActive].addTime(end - start);

            state = newState;
            logger.addStep(*step);
        }
    } catch(AgentSurrenderedException& ex) {
        summary.whoWon = static_cast<unsigned short>(1 - ex.id);
    }
    return summary;
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
