#ifndef REVELATION_HEURISTIC_HPP
#define REVELATION_HEURISTIC_HPP

#include "../step.hpp"

class Heuristic {
public:
    using Value = float;
protected:
    virtual Value evaluateMoveStep(int playerId, const State& oldState, const MoveStep& step) const = 0;
    virtual Value evaluateActionStep(int playerId, const State& oldState, const ActionStep& step) const = 0;
public:
    virtual ~Heuristic() = default;
    Value evaluateStep(int myId, const State& oldState, const Step& step) const;
    virtual Value evaluateMaxForState(int playerId, const State& state, unsigned short nbTurnsRemaining) const = 0;
};

class PowerTimesToughnessHeuristic final : public Heuristic {
protected:
    Value evaluateMaxForState(int playerId, const State& state, unsigned short nbTurnsRemaining) const final {
        int myMaxAtk = 0;
        for(auto unit : state.units[ playerId ]){ //units are sorted by max atk, so the first in the list is the one with the highest atk
            if(not isDead(unit)){
                myMaxAtk = unit->maxAtk;
                break;
            }
        }
        int nbMaxDamage = myMaxAtk * nbTurnsRemaining;
        Value heurVal = 0;
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
    Value evaluateMoveStep(int playerId, const State& oldState, const MoveStep& step) const final;
    Value evaluateActionStep(int playerId, const State& oldState, const ActionStep& step) const final;
};

#endif //REVELATION_HEURISTIC_HPP
