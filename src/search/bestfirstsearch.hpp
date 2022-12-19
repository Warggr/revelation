#ifndef REVELATION_BESTFIRSTSEARCH_HPP
#define REVELATION_BESTFIRSTSEARCH_HPP

#include "search.hpp"
#include <queue>

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

    std::tuple<int, int> asTuple() override {
        return { maxDepth / 2 + maxDepth % 2, maxDepth / 2 };
    }
    bool addEndState(const State& state, const DecisionList& decisions, Heuristic::Value heurVal, const ProcessContext& pc) override {
        SearchPolicy::addEndState(std::move(state), decisions, heurVal, pc);
        if(currentDepth == maxDepth or pc.isInterrupted()){
            return true;
        } else {
            currentDepth++;
            addChild({state, decisions, heurVal});
        }
        return false;
    }
};

#endif //REVELATION_BESTFIRSTSEARCH_HPP
