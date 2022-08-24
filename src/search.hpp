#ifndef REVELATION_SEARCH_HPP
#define REVELATION_SEARCH_HPP

#include "state.hpp"
#include "search/heuristic.hpp"
#include <limits>

struct DecisionList{
    ActionOrResource draw;
    DiscardDecision discard;
    MoveDecision moves[2];
    AbilityDecision ability;
    ActionDecision action;
    std::vector<unsigned int> specialActions;
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

class SearchPolicy {
protected:
    DecisionList bestMoves;
    State bestState;
    const Heuristic& heuristic;
    Heuristic::Value maxHeur, worstOpponentsHeuristic;
public:
    Heuristic::Value maxHeurAllowed = std::numeric_limits<float>::max();

    SearchPolicy(const Heuristic& heuristic): heuristic(heuristic) {};
    virtual ~SearchPolicy() = default;
    /* SearchPolicy specifies the search-node container (e.g. LIFO stack or priority queue) */
    virtual Container<SearchNode>& getContainer() = 0;
    void planAhead(const State& state);
    virtual bool addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal){
        if(heurVal > maxHeur){
            bestMoves = decisions;
            bestState = std::move(state);
            maxHeur = heurVal;
        }
        return false;
    }
    virtual std::tuple<State, DecisionList, Heuristic::Value> getResults() {
        return std::make_tuple(bestState, bestMoves, maxHeur);
    }

    virtual std::tuple<unsigned, unsigned> asTuple() = 0;
    /* Callbacks that some implementations use */
    virtual void informNbChildren(unsigned int nbChildren, Timestep timestepLevel){ (void)nbChildren; (void)timestepLevel; }
};

class SearchAgent: public Agent {
    SearchPolicy* searchPolicy;
    DecisionList plans {};
    unsigned short int currentSpecialAction = 0;
public:
    SearchAgent(unsigned int myId, SearchPolicy* policy): Agent(myId), searchPolicy(policy) {};
    //~SearchAgent() { delete searchPolicy; }
    ActionOrResource getDrawAction(const State&) override { return plans.draw; }
    DiscardDecision getDiscard(const State&) override { return plans.discard; }
    MoveDecision getMovement(const State&, unsigned nb) override { return plans.moves[nb]; }
    AbilityDecision getAbility(const State&) override { return plans.ability; }
    ActionDecision getAction(const State&) override { return plans.action; }
    unsigned int getSpecialAction(const State&, Effect&) override {
        return plans.specialActions[currentSpecialAction++];
    }

    void onBegin(const State &state) override;
};

unsigned int pushChildStates(const SearchNode& stackFrame, Container<SearchNode>& putback, const Heuristic& heuristic );

#endif //REVELATION_SEARCH_HPP
