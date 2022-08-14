#include "heuristic.hpp"
#include "../step.hpp"
#include "../state.hpp"

Heuristic::Value Heuristic::evaluateStep( int myId, const State& oldState, const Step& step) const {
    auto* actionStep = dynamic_cast<const ActionStep*>(&step);
    if(actionStep)
        return evaluateActionStep(myId, oldState, *actionStep);

    auto* moveStep = dynamic_cast<const MoveStep*>(&step);
    if(moveStep)
        return evaluateMoveStep(myId, oldState, *moveStep);

    return 0;
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateMaxForState(int playerId, const State& state, unsigned short nbTurnsRemaining) const {
    int myMaxAtk = 0;
    for(auto unit : state.units[ playerId ]){ //units are sorted by max atk, so the first in the list is the one with the highest atk
        if(not isDead(unit)){
            myMaxAtk = unit->maxAtk;
            break;
        }
    }
    int nbMaxDamage = myMaxAtk * nbTurnsRemaining;
    Heuristic::Value heurVal = 0;
    for(auto enemy : state.units[ 1 - playerId ]){
        if(not isDead(enemy)){
            if(enemy->HP < nbMaxDamage){
                heurVal += enemy->maxAtk * ( enemy->HP + enemy->maxHP );
                nbMaxDamage -= enemy->HP;
            } else {
                heurVal += enemy->maxAtk * nbMaxDamage;
                break;
            }
        }
    }
    return heurVal;
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateActionStep(int myId, const State& oldState, const ActionStep& step) const {
    (void) myId;
    if(step.cardLost == ActionCard::DEFENSE)
        return 50 * oldState.getBoardFieldDeref( step.subject )->maxAtk;
    else {
        const Character* obj = oldState.getBoardFieldDeref( step.object );
        Heuristic::Value ret = step.lostHP * obj->maxAtk;
        if(step.del)
            ret += obj->maxAtk * obj->maxHP;
        return ret;
    }
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateMoveStep(int, const State&, const MoveStep&) const {
    return -1;
}
