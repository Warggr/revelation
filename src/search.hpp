#ifndef REVELATION_SEARCH_HPP
#define REVELATION_SEARCH_HPP

#include "state.hpp"
#include "search/heuristic.hpp"

struct DecisionList{
    ActionOrResource draw;
    MoveDecision moves[2];
    AbilityDecision ability;
    ActionDecision action;
};

class ProgressLogger{
public:
    virtual ~ProgressLogger() = default;
    virtual void enterTurn() = 0;
    virtual void exitTurn() = 0;
    virtual void enter(Timestep timestep, unsigned nbChildren) = 0;
    virtual void exit(Timestep timestep) = 0;
    virtual void message(const char* msg) const;
    virtual void message(const char* msg, float nb) const;
};

struct SearchNode {
    State state;
    DecisionList decisions;
    Heuristic::Value heurVal;

    SearchNode(State state, DecisionList decisions, float heurVal ):
        state(std::move(state)), decisions(std::move(decisions)), heurVal(heurVal)
        {};
    [[nodiscard]] SearchNode copy(State newState, Heuristic::Value heuristicIncrement) const {
        return { std::move(newState), decisions, heurVal + heuristicIncrement };
    }
};

template<typename T>
class Container{
public:
    virtual ~Container() = default;
    virtual void addChild(const T& child) = 0;
    virtual bool hasChildren() = 0;
    virtual T popChild() = 0;
};

class SearchPolicy;

class SearchAgent: public Agent {
    SearchPolicy* searchPolicy;
    DecisionList plans;
public:
    ActionOrResource getDrawAction(const State&) override { return plans.draw; }
    MoveDecision getMovement(const State&, unsigned nb) override { return plans.moves[nb]; }
    AbilityDecision getAbility(const State&) override { return plans.ability; }
    ActionDecision getAction(const State&) override { return plans.action; }

    void onBegin(const State &state) override;
};

class SearchPolicy {
protected:
    DecisionList bestMoves;
    State bestState;
    const Heuristic& heuristic;
    Heuristic::Value maxHeur, worstOpponentsHeuristic;
public:
    SearchPolicy(const Heuristic& heuristic): heuristic(heuristic) {};
    virtual ~SearchPolicy() = default;
    /* SearchPolicy specifies the search-node container (e.g. LIFO stack or priority queue) */
    virtual Container<SearchNode>& getContainer() = 0;
    void planAhead(const State& state, /*ProgressLogger& logger,*/ Heuristic::Value maxHeurAllowed = std::numeric_limits<float>::max());
    virtual void addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal){
        if(heurVal > maxHeur){
            bestMoves = decisions;
            bestState = std::move(state);
            maxHeur = heurVal;
        }
    }
    virtual std::tuple<State, DecisionList, Heuristic::Value> getResults() {
        return std::make_tuple(bestState, bestMoves, maxHeur);
    }

    virtual std::tuple<unsigned, unsigned> asTuple() = 0;
    /* Callbacks that some implementations use */
    virtual void informNbChildren(unsigned int nbChildren, Timestep timestepLevel){ (void)nbChildren; (void)timestepLevel; }
};

unsigned int pushChildStates(const SearchNode& stackFrame, Container<SearchNode>& putback, const Heuristic& heuristic );

#endif //REVELATION_SEARCH_HPP
