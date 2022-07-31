#ifndef REVELATION_SEARCH_HPP
#define REVELATION_SEARCH_HPP

#include "state.hpp"

struct DecisionList{
    ActionOrResource draw;
    MoveDecision moves[2];
    AbilityDecision ability;
    ActionDecision action;
};

class Heuristic {
public:
    using Value = float;

    virtual Value evaluateStep(int myId, const State& oldState, const Step& step) const = 0;
    virtual Value evaluateMaxForState(int playerId, const State& state, unsigned short nbTurnsRemaining) const = 0;
};

struct StackFrame {
    State state;
    DecisionList decisions;
    Heuristic::Value heurVal;

    StackFrame(const State& state, const DecisionList& decisions, float heurVal ):
        state(state), decisions(decisions), heurVal(heurVal)
        {};
    StackFrame copy(const State& newState, Heuristic::Value heuristicIncrement) const {
        return StackFrame( newState, decisions, heurVal + heuristicIncrement );
    }
};

class SearchPolicy {
protected:
    bool finished = false;
public:
    bool isFinished() const { return finished; }
    virtual SearchPolicy* enterOpponentsTurn() = 0;
    virtual SearchPolicy* enterOwnTurn(){
        return nullptr; // depth policies by default do not allow simulating my own turn after the opponent's simulation
    }
    virtual std::tuple<unsigned short, unsigned short> asTuple() = 0;
    virtual void addChild(const StackFrame& child) = 0;
    virtual bool hasChildren() const = 0;
    virtual StackFrame popChild() = 0;
    virtual void informNbChildren(unsigned int nbChildren, Timestep timestepLevel){ (void)nbChildren; (void)timestepLevel; }
};

unsigned int pushChildStates(const StackFrame& stackFrame, SearchPolicy& putback, const Heuristic& heuristic);

#endif //REVELATION_SEARCH_HPP
