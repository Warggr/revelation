#include "game.hpp"
#include "agent.hpp"
#include "gameplay/state.hpp"
#include "logging/logger.hpp"
#include <random>

GeneratorSeed getRandom(){
    return std::random_device()();
}

Game::Game(std::array<const Team*, 2> teams, std::array<Agent*, 2> agents, GeneratorSeed seed):
    state(State::createStart(teams, Generator(seed))), teams(teams), agents(agents)
    , logger(state, teams, seed)
{
}

GameSummary Game::play() {
    try { //A disconnected agent should be able to throw an exception and kill the game.
        unsigned short int winner = 0;
        unsigned int nTurn = 0;
        while ((winner = state.getWinner()) == 0 and nTurn < 30000) {
            nTurn++;
            if (state.timestep == Timestep::BEGIN)
                agents[state.iActive]->onBegin(state);
            auto[newState, step] = state.advance(*agents[state.iActive], *agents[1 - state.iActive]);
            state = newState;
            logger.addStep(*step);
        }
        return { winner };
    } catch(AgentSurrenderedException& ex) {
        return { static_cast<unsigned short>(1 - ex.id) };
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
