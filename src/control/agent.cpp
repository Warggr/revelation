#include "agent.hpp"
#include "gameplay/state.hpp"
#include "setup/units_repository.hpp"

SimpleAgent SimpleAgent::instance;

Agent& Agent::getFallback(){ return SimpleAgent::getInstance(); }

const Player& Agent::getMyPlayer(const State& state) const {
    return state.players[ myId ];
}

const Team& SimpleAgent::getTeam(const UnitsRepository& repo) {
    return repo.getTeams().begin()->second;
}

DiscardDecision SimpleAgent::getDiscard(const State&){
    return DiscardDecision();
}

MoveDecision SimpleAgent::getMovement(const State&, unsigned int){
    return MoveDecision::pass();
}

ActionDecision SimpleAgent::getAction(const State&){
    return ActionDecision::pass();
}

unsigned int SimpleAgent::getSpecialAction(const State&, const Effect&){
    return 0;
}
