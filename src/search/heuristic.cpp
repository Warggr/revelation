#include "heuristic.hpp"
#include "gameplay/step.hpp"
#include "gameplay/state.hpp"

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
    for(const auto& unit : state.units[ playerId ]){ //units are sorted by max atk, so the first in the list is the one with the highest atk
        if(not isDead(unit)){
            myMaxAtk = unit->im.maxAtk;
            break;
        }
    }
    int nbMaxDamage = myMaxAtk * nbTurnsRemaining;
    Heuristic::Value heurVal = 0;
    for(const auto& enemy : state.units[ 1 - playerId ]){
        if(not isDead(enemy)){
            if(enemy->HP < nbMaxDamage){
                heurVal += enemy->im.maxAtk * ( enemy->HP + enemy->im.maxHP );
                nbMaxDamage -= enemy->HP;
            } else {
                heurVal += enemy->im.maxAtk * nbMaxDamage;
                break;
            }
        }
    }
    return heurVal;
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateActionStep(int myId, const State& oldState, const ActionStep& step) const {
    (void) myId;
    if(step.isPass())
        return 0;
    if(step.cardLost == ActionCard::DEFENSE)
        return 50 * oldState.getBoardFieldDeref( step.subject )->im.maxAtk;
    else {
        const Character* obj = oldState.getBoardFieldDeref( step.object );
        Heuristic::Value ret = step.atk.lostHP * obj->im.maxAtk;
        if(step.del)
            ret += obj->im.maxAtk * obj->im.maxHP;
        return ret;
    }
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateMoveStep(int, const State&, const MoveStep&) const {
    return -1;
}
