#include "heuristic.hpp"
#include "../step.hpp"
#include "../state.hpp"

Value Heuristic::evaluateStep( int myId, const State& oldState, const Step& step) const {
    auto* actionStep = dynamic_cast<ActionStep*>(&step);
    if(actionStep){
        return evaluateActionStep(*actionStep);
    }
    auto* moveStep = dynamic_cast<MoveStep*>(&step);
    if(moveStep)
        return evaluateMoveStep(*moveStep);

    return 0;
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateActionStep(int myId, const State& oldState, const ActionStep& step) const {
    if(step.card == ActionCard::DEFENSE)
        return 50 * oldState.getBoardFieldDeref( step.subjectPos ).maxAtk;
    else {
        character* obj = oldState.getBoardFieldDeref( step.objectPos );
        Heuristic::Value ret = step.lostLife * obj.maxAtk;
        if(step.delete)
            ret += obj.maxAtk * obj.maxHP;
        return ret;
    }
}

Heuristic::Value PowerTimesToughnessHeuristic::evaluateMoveStep(int, const State&, const MoveStep&) const {
    return -1;
}
