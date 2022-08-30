#ifndef REVELATION_DEPTHFIRSTSEARCH_HPP
#define REVELATION_DEPTHFIRSTSEARCH_HPP

#include "../search.hpp"
#include <cassert>

/**
 * In DFS, it is possible to determine when we leave a branch of the search tree.
 * This enables better logging.
 */
class ProgressLogger{
public:
    virtual ~ProgressLogger() = default;
    virtual void enterTurn() = 0;
    virtual void exitTurn() = 0;
    virtual void enter(Timestep timestep, unsigned nbChildren);
    virtual void exit(Timestep timestep);
    virtual void message(const char* msg) const;
    virtual void message(const char* msg, float nb) const;
};

class LifoStack;

/*
 * Abstract class for different kinds of DFS-like search policies.
 * Classes inheriting from this can specify until which depth to go by overriding @method enterOpponentsTurn.
 */
class DepthFirstSearch : public SearchPolicy {
protected:
    ProgressLogger& logger;
    virtual SearchPolicy* enterOpponentsTurn() = 0;
public:
    DepthFirstSearch(const Heuristic& heuristic, ProgressLogger& logger): SearchPolicy(heuristic), logger(logger) {};
    virtual ~DepthFirstSearch() = default;
    void informNbChildren(unsigned nbChildren, Timestep timestep) override { logger.enter(timestep, nbChildren); }
    bool addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) override;

    friend class LifoStack;
};

class LifoStack: public Container<SearchNode> {
    struct MySearchNode {
        SearchNode frame;
        bool pass2;
        MySearchNode(SearchNode frame, bool pass2): frame(std::move(frame)), pass2(pass2) {};
    };
    std::vector<MySearchNode> stack;
    DepthFirstSearch* parent;
public:
    LifoStack(DepthFirstSearch* parent): parent(parent) {};
    void addChild(const SearchNode& child) override {
        stack.emplace_back( child, false );
    }
    bool hasChildren() override {
        while(not stack.empty() and stack.back().pass2){
            parent->logger.exit(stack.back().frame.state.timestep);
            stack.pop_back();
        }
        return not stack.empty();
    }
    SearchNode popChild() override {
        auto [ frame, pass2 ] = stack.back();
        assert(not pass2);
        stack.back().pass2 = true;
        return frame;
    }
};

/**
 * Some DFS do not have a fixed depth and always return themselves as enterOpponentsTurn.
 * Since they are called on multiple search levels, they need a different container for each level.
 */
class PerpetualDFS: public DepthFirstSearch {
    std::vector<LifoStack> containers;
protected:
    SearchPolicy* enterOpponentsTurn() override { return this; }
public:
    PerpetualDFS(const Heuristic& heuristic, ProgressLogger& logger): DepthFirstSearch(heuristic, logger) {};
    void init(const State&) override { containers.emplace_back(this); }
    void exit() override { containers.pop_back(); }
    Container<SearchNode>& getContainer() override {
        assert(not containers.empty());
        return containers.back();
    }
};
// All other DFS implementations
class NormalDFS: public DepthFirstSearch {
    LifoStack container;
public:
    NormalDFS(const Heuristic& heuristic, ProgressLogger& logger): DepthFirstSearch(heuristic, logger), container(this) {};
    Container<SearchNode>& getContainer() override { return container; }
};

class StaticDFS : public NormalDFS {
    SearchPolicy* opponentsTurn = nullptr;
protected:
    SearchPolicy* enterOpponentsTurn() override { return opponentsTurn; }
public:
	StaticDFS(const Heuristic& heuristic, ProgressLogger& logger): NormalDFS(heuristic, logger) {};
    template<typename PolicyType>
    PolicyType* setOpponentsTurn(PolicyType* t){ opponentsTurn = t; return t; }

    std::tuple<int, int> asTuple() override {
        int my = 0, your = 0;
        if(opponentsTurn)
            std::tie(your, my) = opponentsTurn->asTuple();
        return std::make_tuple(my + 1, your);
    }
};

class UntilSomeoneDiesDFS : public PerpetualDFS {
    std::array<unsigned short int, 2> nbAliveUnits;
public:
    UntilSomeoneDiesDFS(const Heuristic& heuristic, ProgressLogger& logger): PerpetualDFS(heuristic, logger) {};
    void init(const State& state) override;
    std::tuple<int, int> asTuple() override { return std::make_tuple(-1, -1); }
    bool addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) override;
};

class AdaptiveDepthFirstSearch : public NormalDFS {
    constexpr static int usedLevelsMap[] = { 0, 1, 1, -1, 2, 3 };
    constexpr static int nbUsedLevels = 4;

    const unsigned maxNodes;
    unsigned nodes;
    int currentLevel;
    unsigned sumChildren[nbUsedLevels]{0};
    unsigned nbChildren[nbUsedLevels]{0};
    unsigned currentChildrenCount[nbUsedLevels]{0};
    AdaptiveDepthFirstSearch* opponentsTurn = nullptr;

    unsigned estimateNbBranches();
protected:
    SearchPolicy* enterOpponentsTurn() override;
    void init(const State& state) override;
public:
    AdaptiveDepthFirstSearch(const Heuristic& heuristic, ProgressLogger& logger, unsigned maxNodes = 1000000): NormalDFS(heuristic, logger),
        maxNodes(maxNodes) {};
    ~AdaptiveDepthFirstSearch(){ if(opponentsTurn) delete opponentsTurn; }
    void informNbChildren(unsigned nbChildren, Timestep timestep) override;
    std::tuple<int, int> asTuple() override;
};

#endif //REVELATION_DEPTHFIRSTSEARCH_HPP
