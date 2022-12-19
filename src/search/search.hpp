#ifndef REVELATION_SEARCH_HPP
#define REVELATION_SEARCH_HPP

#include "heuristic.hpp"
#include "control/agent.hpp"
#include "gameplay/state.hpp"
#include <limits>
#include <utility>
#include <vector>
#include <memory>

template<typename T>
using uptr = std::unique_ptr<T>;

struct DecisionList{
#ifndef NDEBUG
    bool initialized = false;
    State beforeDiscard, beforeMove[2], beforeAbility, beforeAction;
#endif
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
    [[nodiscard]] SearchNode copy(const State& newState, Heuristic::Value heuristicIncrement) const {
        return { newState, decisions, heurVal + heuristicIncrement };
    }
};

template<typename T>
class Container{
public:
    virtual ~Container() = default;
    virtual void addChild(const T& child) = 0;
    virtual bool hasChildren() = 0;
    bool empty(){ return not hasChildren(); }
    virtual T popChild() = 0;
    void clear(){ while(hasChildren()) popChild(); }
};

/**
 * This class allows to search amongst child states of a start state.
 * It is kept very general and allows derived classes to specify different ways of searching by overriding different callback methods.
 * By default, it searches only the states until the end of the turn, and calls the callback method addEndState on each of them.
 * In what order the states are searched depends on the container returned by getContainer(). For example, depth-first search uses a LIFO stack.
 */
class SearchPolicy {
protected:
    DecisionList bestMoves;
    State bestState;
    const Heuristic& heuristic;
    Heuristic::Value maxHeur, worstOpponentsHeuristic;
#ifdef CONFIG_AGENT_IS_INTERRUPTIBLE
    std::vector<std::pair<DecisionList, State>> interruptedDecisionsCallStack;
#endif
#ifndef NDEBUG
    bool reachedEndState;
    int nbTimesEntered = 0;

    //callback functions that can be overridden
    virtual void init(const State&){
        nbTimesEntered++;
    };
    virtual void exit(){
        nbTimesEntered--;
        assert(nbTimesEntered >= 0);
    };
public:
    int getNbTimesEntered() const { return nbTimesEntered; }
#else
    virtual void init(const State&){};
    virtual void exit(){};
public:
#endif
    Heuristic::Value maxHeurAllowed = std::numeric_limits<float>::max();

    SearchPolicy(const Heuristic& heuristic): heuristic(heuristic) {};
    virtual ~SearchPolicy() = default;
    virtual Container<SearchNode>& getContainer() = 0;

    void planAhead(const State& state, const ProcessContext& processContext);

    //Return false by default, and true if we should cut off the search directly
    virtual bool addEndState(const State& state, const DecisionList& decisions, Heuristic::Value heurVal, const ProcessContext&){
        if(heurVal > maxHeur){
#ifndef NDEBUG
            reachedEndState = true;
#endif
            bestMoves = decisions;
            bestState = state;
            maxHeur = heurVal;
        }
        assert(reachedEndState);
        return false;
    }
    virtual bool addWinState(const State& state, const DecisionList& decisions){
#ifndef NDEBUG
        reachedEndState = true;
#endif
        bestMoves = decisions;
        bestState = state;
        maxHeur = std::numeric_limits<float>::max();
        return true;
    }
    virtual std::tuple<State, DecisionList, Heuristic::Value> getResults() {
        assert(reachedEndState);
        assert(bestMoves.initialized);
        return std::make_tuple(bestState, bestMoves, maxHeur);
    }

    virtual std::tuple<int, int> asTuple() = 0;
    /* Callbacks that some implementations use */
    virtual void informNbChildren(unsigned int nbChildren, Timestep timestepLevel){ (void)nbChildren; (void)timestepLevel; }
};

class SearchAgent: public Agent {
    uptr<SearchPolicy> searchPolicy;
    uptr<Heuristic> heuristic;
    DecisionList plans {};
    unsigned short int currentSpecialAction = 0;
public:
    SearchAgent(unsigned int myId, uptr<SearchPolicy> policy, uptr<Heuristic> heuristic)
    : Agent(myId), searchPolicy(std::move(policy)), heuristic(std::move(heuristic)) {};
    DiscardDecision getDiscard(const State&) override;
    MoveDecision getMovement(const State&, unsigned nb) override;
    AbilityDecision getAbility(const State&) override;
    ActionDecision getAction(const State&) override;
    unsigned int getSpecialAction(const State&, const Effect&) override {
        return plans.specialActions[currentSpecialAction++];
    }

    const Team& getTeam(const UnitsRepository& repo) override;
    void onBegin(const State& state) override;
};

unsigned int pushChildStates(const SearchNode& stackFrame, Container<SearchNode>& putback, const Heuristic& heuristic );

#endif //REVELATION_SEARCH_HPP
