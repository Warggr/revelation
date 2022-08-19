#ifndef REVELATION_HEURISTIC_HPP
#define REVELATION_HEURISTIC_HPP

#include "../step_impl.hpp"

class State;

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
    Value evaluateMaxForState(int playerId, const State& state, unsigned short nbTurnsRemaining) const final;
    Value evaluateMoveStep(int playerId, const State& oldState, const MoveStep& step) const final;
    Value evaluateActionStep(int playerId, const State& oldState, const ActionStep& step) const final;
};

#endif //REVELATION_HEURISTIC_HPP
