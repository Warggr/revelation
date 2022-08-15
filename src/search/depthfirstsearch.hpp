#ifndef REVELATION_DEPTHFIRSTSEARCH_HPP
#define REVELATION_DEPTHFIRSTSEARCH_HPP

#include "../search.hpp"
#include <queue>

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

/*
 * Abstract class for different kinds of DFS-like search policies.
 * Classes inheriting from this can specify until which depth to go by overriding @method enterOpponentsTurn.
 */
class DepthFirstSearch : public SearchPolicy, public Container<SearchNode> {
    struct MySearchNode {
        SearchNode frame;
        bool pass2;
        MySearchNode(SearchNode frame, bool pass2): frame(std::move(frame)), pass2(pass2) {};
    };
    std::vector<MySearchNode> stack;
protected:
    ProgressLogger& logger;
    virtual SearchPolicy* enterOpponentsTurn() = 0;
public:
/* DFS behaves as a container, more precisely as a LIFO stack */
    void addChild(const SearchNode& child) override {
        stack.emplace_back( child, false );
    }
    bool hasChildren() override {
        while(not stack.empty() and stack.back().pass2){
            logger.exit(stack.back().frame.state.timestep);
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

    DepthFirstSearch(const Heuristic& heuristic, ProgressLogger& logger): SearchPolicy(heuristic), logger(logger) {};
    virtual ~DepthFirstSearch() = default;
    Container<SearchNode>& getContainer() override { return *this; }
    void informNbChildren(unsigned nbChildren, Timestep timestep) override { logger.enter(timestep, nbChildren); }

    bool addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) override;
};

class StaticDepthFirstSearch : public DepthFirstSearch {
    SearchPolicy* opponentsTurn = nullptr;
protected:
    SearchPolicy* enterOpponentsTurn() override { return opponentsTurn; }
public:
	StaticDepthFirstSearch(const Heuristic& heuristic, ProgressLogger& logger): DepthFirstSearch(heuristic, logger) {};
    template<typename PolicyType>
    PolicyType* setOpponentsTurn(PolicyType* t){ opponentsTurn = t; return t; }

    std::tuple<unsigned, unsigned> asTuple() override {
        unsigned my = 0, your = 0;
        if(opponentsTurn)
            std::tie(your, my) = opponentsTurn->asTuple();
        return std::make_tuple(my + 1, your);
    }
};

class AdaptiveDepthFirstSearch : public DepthFirstSearch {
    constexpr static int usedLevelsMap[] = { 0, -1, 1, -1, 2, 3 };
    constexpr static int nbUsedLevels = 4;

    unsigned maxNodes;
    unsigned nodes;
    int currentLevel;
    unsigned sumChildren[nbUsedLevels]{0};
    unsigned nbChildren[nbUsedLevels]{0};
    unsigned currentChildrenCount[nbUsedLevels]{0};
    AdaptiveDepthFirstSearch* opponentsTurn = nullptr;

    unsigned estimateNbBranches();
public:
    AdaptiveDepthFirstSearch(const Heuristic& heuristic, ProgressLogger& logger, unsigned maxNodes = 1000000): DepthFirstSearch(heuristic, logger),
        maxNodes(maxNodes), nodes(1), currentLevel(-1) {}
    ~AdaptiveDepthFirstSearch(){ if(opponentsTurn) delete opponentsTurn; }
    void informNbChildren(unsigned nbChildren, Timestep timestep) override;
    SearchPolicy* enterOpponentsTurn() override;
    std::tuple<unsigned, unsigned> asTuple() override;
};

class GreedyBestFirstSearchAgent: public SearchPolicy, public Container<SearchNode> {
    struct MyQueueFrame{
        SearchNode stackFrame;
        unsigned depth;

        MyQueueFrame( SearchNode frame, unsigned depth ): stackFrame(std::move(frame)), depth(depth) {};
        bool operator<(const MyQueueFrame& other) const { return stackFrame.heurVal < other.stackFrame.heurVal; }
    };
    std::priority_queue<MyQueueFrame> queue;
    unsigned maxDepth;
    unsigned currentDepth = 0;
public:
    explicit GreedyBestFirstSearchAgent(const Heuristic& heuristic, unsigned maxDepth): SearchPolicy(heuristic), maxDepth(maxDepth) {};

    void addChild(const SearchNode& child) override {
        queue.push( MyQueueFrame( child, currentDepth ) );
    }
    bool hasChildren() override { return not queue.empty(); }
    SearchNode popChild() override {
        auto [ node, depth ] = queue.top();
        queue.pop();
        currentDepth = depth;
        return node;
    }
    Container<SearchNode>& getContainer() override { return *this; }

    std::tuple<unsigned int, unsigned int> asTuple() override {
        return { maxDepth / 2 + maxDepth % 2, maxDepth / 2 };
    }
    bool addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) override {
        if(currentDepth == maxDepth){
            SearchPolicy::addEndState(std::move(state), decisions, heurVal);
        } else {
            currentDepth++;
            addChild({state, decisions, heurVal});
        }
        return false;
    }
};

class SimpleIndentLogger: public ProgressLogger{
    unsigned short int indent = 0;
public:
    void enterTurn() override { indent += 1; }
    void exitTurn() override { indent -= 1; }
    void message( const char* msg ) const override;
    void message( const char* msg, float x ) const override;
};

class ProgressBar : public ProgressLogger{
    std::vector<std::vector<int>> progressStack;
    std::vector<int> progress;
public:
    void message( const char* ) const override {};
    void message( const char*, float ) const override {};
    void enter(Timestep timestep, unsigned nbChildren) override;
    void exit(Timestep timestep) override;
    void enterTurn() override;
    void exitTurn() override;
};

#endif //REVELATION_DEPTHFIRSTSEARCH_HPP
