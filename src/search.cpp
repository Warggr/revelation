#include "search.hpp"
#include "state.hpp"
#include "agent.hpp"
#include "position.hpp"
#include <tuple>
#include <unordered_set>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <queue>
#include <limits>

void SearchAgent::onBegin(const State &state) {
//    ProgressLogger* logger; //TODO
    searchPolicy->planAhead(state /*, *logger*/);
    State newState; Heuristic::Value heurVal;
    std::tie(newState, plans, heurVal) = searchPolicy->getResults();
}

void SearchPolicy::planAhead(const State& startState, /*ProgressLogger& logger,*/ Heuristic::Value maxHeurAllowed){
    /*
    Constructs all descendants of @param state up to a level defined by @param depthPolicy.
    Specify @param maxHeurAllowed if you want the function to return directly if it reaches a certain heuristic.
    */
    (void) maxHeurAllowed; //TODO
    //searchPolicy->getLogger().enterTurn();
    //nbPaths = 0
    maxHeur = std::numeric_limits<float>::min();
    worstOpponentsHeuristic = std::numeric_limits<float>::max();

    Container<SearchNode>& container = getContainer();
    container.addChild( SearchNode(startState, DecisionList(), 0) );

    while(container.hasChildren()){
        SearchNode node = container.popChild();

        const State& state = node.state;
        if(state.timestep != Timestep::ACTED){
            /*unsigned nbChildren =*/ pushChildStates(node, container);
            //searchPolicy->getLogger().enter(state.timestep, nbChildren);
            //searchPolicy->informNbChildren(nbChildren, state.timestep);
        } else {
            auto [ newState, step ] = state.beginTurn();
            //searchPolicy->getLogger().enter(state.timestep, 1);
            addEndState(newState, node.decisions, node.heurVal);

          /*  SearchPolicy* subDepth = enterOpponentsTurn();
            if(subDepth){
                auto [ nbTurnsMe, nbTurnsOpponent ] = subDepth->asTuple();
                // we"re using state.iActive and not newState.iActive because in newState, iActive has already switched to the opponent
                Heuristic::Value max_bound = node.heurVal + heuristic->evaluateMaxForState(newState, 1 - state.iActive, nbTurnsOpponent);
                Heuristic::Value min_bound = node.heurVal - heuristic->evaluateMaxForState(newState, state.iActive, nbTurnsMe - 1);
                if(min_bound >= maxHeurAllowed){ // we are in any case over the maximal allowed value
                    //searchPolicy->exit();
                    return std::make_tuple(State::invalid(), DecisionList(), min_bound);
                }
                if(max_bound <= maxHeur) // in any case, we already had a better solution
                    continue;

                subDepth->init(newState, node.decisions, node.heurVal);
            }*/
        }
    }
    //print(nbPaths, "possible futures found");
}

using HashKey = int;

static_assert ( FULL_BOARD_WIDTH < (1 << 4) );

unsigned char hashPosition( const position& pos ){ //max. 4 bits for column and 1 for row
    return 2 * pos.column + pos.column;
}

HashKey hashBoard( const std::array<character*, 6>& units ){
    int retVal = 0;
    for(unsigned i = 0; i<6; i++)
        retVal += (isDead(units[i]) ? hashPosition(units[i]->pos) + 1 : 0) << (5*i);
    return retVal;
}

unsigned pushChildStates(const SearchNode& stackFrame, Container<SearchNode>& putBack, const Heuristic& heuristic){
    /*
    Creates all children states of state (the first element in the @param stackFrame) and adds them into the data structure @param putBack.
    @returns the number of children inserted this way.
    */
    const State& state = stackFrame.state;
    switch(state.timestep){
    case Timestep::BEGIN:{
        auto decision = ActionOrResource::ACTION;
        auto [ newState, step ] = state.stepDraw( decision );
        SearchNode newFrame = stackFrame.copy(newState, heuristic.evaluateStep( state.iActive, state, *step ));
        newFrame.decisions.draw = decision;
        putBack.addChild( newFrame );
        return 1;
    }

    case Timestep::DISCARDED:{
        std::unordered_set<HashKey> allPossibleMoves;
        //nbChildrenFiltered = 0

        using SubStackFrame = std::tuple<State, std::array<MoveDecision, 2>, Heuristic::Value>;

        std::vector<SubStackFrame> oldStates;
        oldStates.emplace_back( state, std::array<MoveDecision, 2>(), stackFrame.heurVal );
        for(unsigned i = 0; i<3; i++){
            std::vector<SubStackFrame> newStates;
            for( const auto& [ subState, decisions, heurValOld ] : oldStates ){
                Heuristic::Value heurVal = heurValOld;
                HashKey stateid = hashBoard(subState.units[subState.iActive]);
                //print("with", [ printDecision(decision) if decision else None for decision in decisions ], ", stateid would be ", stateid);
                if( allPossibleMoves.find(stateid) != allPossibleMoves.end() ){ //already in allPossibleMoves
                    //nbChildrenFiltered += 1
                    continue;
                }
                auto newStackFrame = stackFrame.copy(subState, 0);
                if(i != 2){
                    MoveDecision decision = MoveDecision::pass();
                    auto [ newSubState, step ] = subState.stepMov(decision);
                    assert(step->isPass());
                    newStackFrame = newStackFrame.copy( newSubState, heuristic.evaluateStep( state.iActive, newSubState, *step ) );
                    newStackFrame.decisions.moves[i+1] = decision;
                }
                putBack.addChild( newStackFrame );
                allPossibleMoves.insert( stateid );

                if(i != 2){
                    for(auto charSel : subState.units[ subState.iActive ]){
                        if(not isDead(charSel)){
                            auto possibleMovs = subState.allMovementsForCharacter(*charSel);
                            for(const auto& movSel : possibleMovs){
                                std::array<MoveDecision, 2> newDecisions = decisions;
                                newDecisions[i+1] = movSel;
                                auto [ newState, step ] = subState.stepMov( newDecisions[i+1] );
                                heurVal += heuristic.evaluateStep( state.iActive, newState, *step );

                                newStates.emplace_back(newState, newDecisions, heurVal );
                            }
                        }
                    }
                }
            }
            oldStates = std::move(newStates);
        }

        //print("End allMov phase! Same-state filtering kept", len(allPossibleMoves), "out of", nbChildrenFiltered + len(allPossibleMoves) );
        return allPossibleMoves.size();
    }

    case Timestep::MOVEDlast:{
        auto decision = AbilityDecision();
        auto [ newState, step ] = state.stepAbil( decision );
        SearchNode retVal = stackFrame.copy( newState, heuristic.evaluateStep( state.iActive, state, *step ) );
        retVal.decisions.ability = decision;
        putBack.addChild( retVal );
        return 1;
    }

    case Timestep::ABILITYCHOSEN:{
        { // the "pass" decision
            ActionDecision passDecision = ActionDecision::pass();
            auto [ newState, step ] = state.stepAct(passDecision);
            SearchNode newFrame = stackFrame.copy(newState, heuristic.evaluateStep( state.iActive, state, *step ));
            newFrame.decisions.action = passDecision;
            putBack.addChild( newFrame );
            //print("Appending to stack "pass"", len(stack));
        }
        ActionDecision decision;
        unsigned nbChildren = 1; // already one child: the "pass" decision

        for(auto card : state.players[ state.iActive ].getActions()){
            decision.card = card;
            if(card == ActionCard::DEFENSE){
                for(auto subject : state.units[ state.iActive ]){
                    if(not isDead(subject)){
                        decision.subjectPos = subject->pos;
                        auto [ newState, step ] = state.stepAct( decision );
                        SearchNode retVal = stackFrame.copy( newState, heuristic.evaluateStep( state.iActive, state, *step ) );
                        retVal.decisions.action = decision;
                        putBack.addChild( retVal );
                        //print("Appending defense to stack", len(stack));
                        nbChildren += 1;
                    }
                }
            } else {
                auto allPossibleAttacks = state.allAttacks();
                for(auto [ aggressor, victims ] : allPossibleAttacks){
                    for(auto victim : victims){
                        decision.subjectPos = state.units[ state.iActive ][ aggressor->teampos ]->pos;
                        decision.objectPos = state.units[ 1 - state.iActive ][ victim->teampos ]->pos;
                        auto [ newState, step ] = state.stepAct( decision );
                        SearchNode retVal = stackFrame.copy( newState, heuristic.evaluateStep( state.iActive, state, *step ) );
                        retVal.decisions.action = decision;
                        putBack.addChild( retVal );
                        //print("Appending attack to stack", len(stack));
                        nbChildren += 1;
                    }
                }
            }
        }
        return nbChildren;
    }

    default:
        throw std::exception(); //Timestep::ACTED (aka end of turns) are supposed to be handled differently
    }
}

class Stack: public Container<SearchNode> {
    struct MySearchNode {
        SearchNode frame;
        bool pass2;
        MySearchNode(SearchNode frame, bool pass2): frame(std::move(frame)), pass2(pass2) {};
    };
    std::vector<MySearchNode> stack;
public:
    void addChild(const SearchNode& child) override {
        stack.emplace_back( child, false );
    }
    bool hasChildren() override {
        while(stack.back().pass2){
            //logger->exit(stack.back().frame.timestep);
            stack.pop_back();
        }
        return not stack.empty();
    }
    SearchNode popChild() override {
        auto [ frame, pass2 ] = stack.back();
        assert(!pass2);
        return frame;
    }
};

/*
 * Abstract class for different kinds of DFS-like search policies.
 * Classes inheriting from this can specify until which depth to go by overriding @method enterOpponentsTurn.
 */
class DepthFirstSearch : public SearchPolicy {
    Stack stack;
    ProgressLogger* logger;
protected:
    virtual SearchPolicy* enterOpponentsTurn() = 0;
public:
//    ProgressLogger& getLogger() override { return *logger; }
    Container<SearchNode>& getContainer() override { return stack; }
/*    std::tuple<unsigned, unsigned> asTuple() override {
        auto [ nbTurnsOpponent, nbTurnsMe ] = opponentsTurn ? opponentsTurn->asTuple() : std::make_tuple(0, 0);
        return std::make_tuple( nbTurnsMe + 1, nbTurnsOpponent );
    }*/

    void addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) override {
        /* This is depth-first, so we directly go deeper when we enter an end-of-turn state. */
        SearchPolicy* opponentsTurn = enterOpponentsTurn();
        if(opponentsTurn) {
            logger->message("MINNING with cut-off at", worstOpponentsHeuristic);
            opponentsTurn->planAhead(state, /* *logger,*/ worstOpponentsHeuristic);
            auto [ newState, _, heurDiff ] = opponentsTurn->getResults();
            state = newState;

            if (State::isInvalid(state)) { // this happens when the search was better than bestOpponentsHeuristic and was cut off
                logger->message("Search cut off");
                return;
            }
            if (heurDiff < worstOpponentsHeuristic) {
                worstOpponentsHeuristic = heurDiff;
            }
            heurVal -= heurDiff;
            logger->message("Search returned", heurDiff);
        }
        SearchPolicy::addEndState(state, decisions, heurVal);
    }
};

class StaticDepthFirstSearch : public DepthFirstSearch {
    SearchPolicy* opponentsTurn = nullptr;
protected:
    SearchPolicy* enterOpponentsTurn() override { return opponentsTurn; }
public:
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

    unsigned estimateNbBranches(){
        unsigned nbBranches = currentChildrenCount[ currentLevel ];
        for(int i = currentLevel; i != 0; i--){
            nbBranches = ( nbBranches + sumChildren[ i ] ) * ( currentChildrenCount[ i - 1 ] / (nbChildren[i] + 1) ) + 1;
        }
        return nbBranches;
    }
public:
    AdaptiveDepthFirstSearch(unsigned maxNodes = 1000000): maxNodes(maxNodes), nodes(1), currentLevel(-1) {}
    void informNbChildren(unsigned nbChildren, Timestep timestep) override {
        // print(timestepLevel.value, "->", AdaptiveDepthPolicy.usedLevelsMap[ timestepLevel.value ], "->", nbChildren);
        int timestepLevel = usedLevelsMap[ timestep ];
        if(timestepLevel == currentLevel + 1){
            //pass
        } else if(timestepLevel == currentLevel){
            sumChildren[ timestepLevel ] += currentChildrenCount[ timestepLevel ];
            this->nbChildren[ timestepLevel ] += 1;
        } else if(timestepLevel == currentLevel - 1){
            sumChildren[ timestepLevel ] += sumChildren[ currentLevel ] + currentChildrenCount[ currentLevel ] + 1;
            this->nbChildren[ timestepLevel ] += 1;
            sumChildren[ currentLevel ] = 0;
            this->nbChildren[ currentLevel ] = 0;
        } else {
            assert(false);
        }
        currentLevel = timestepLevel;
        currentChildrenCount[ timestepLevel ] = nbChildren;
    }
    SearchPolicy* enterOpponentsTurn() override {
        nodes = estimateNbBranches();
        if(maxNodes <= 1.5 * (nodes * nodes)) return nullptr;
        else return new AdaptiveDepthFirstSearch(maxNodes / nodes);
    }
    std::tuple<unsigned, unsigned> asTuple() override {
        unsigned log = 0;
        unsigned i = maxNodes;
        while(i > 1){
            i = i / nodes - 1;
            log += 1;
        }
        return { log / 2 + log % 2, log / 2 };
    }
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
    explicit GreedyBestFirstSearchAgent(unsigned maxDepth): maxDepth(maxDepth) {};

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
    void addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) override {
        if(currentDepth == maxDepth){
            SearchPolicy::addEndState(std::move(state), decisions, heurVal);
        } else {
            currentDepth++;
            addChild({state, decisions, heurVal});
        }
    }
};

class SimpleIndentLogger: public ProgressLogger{
    unsigned short int indent = 0;
public:
    void enterTurn() override { indent += 1; }
    void exitTurn() override { indent -= 1; }
    void message( const char* msg ) const override {
        for(unsigned i = 0; i < indent; i++) std::cout << "    ";
        std::cout << msg << '\n';
    }
    void message( const char* msg, float x ) const override {
        for(unsigned i = 0; i < indent; i++) std::cout << "    ";
        std::cout << msg << x << '\n';
    }
};

class ProgressBar : public ProgressLogger{
    std::vector<std::vector<int>> progressStack;
    std::vector<int> progress;
public:
    void message( const char* ) const override {};
    void message( const char*, float ) const override {};
    void enter(Timestep timestep, unsigned nbChildren) override {
        if(timestep == Timestep::DISCARDED){
            printf("[%4d\\   1]->", nbChildren);
            progress.push_back(0);
        } else if(timestep == Timestep::MOVEDlast){
            progress[progress.size() - 1] += 1;
            for(int i =0; i<7; i++) printf("\b");
            printf("%4d]->", progress[progress.size() - 1]);
        } else if(timestep == Timestep::ABILITYCHOSEN){
            printf("<%4d\\   1>->", nbChildren);
            progress.push_back(0);
        } else if(timestep == Timestep::ACTED){
            progress[progress.size()-1] += 1;
            for(int i =0; i<7; i++) printf("\b");
            printf("%4d>->", progress[progress.size() - 1]);
        }
    }
    void exit(Timestep timestep) override {
        if(timestep == Timestep::DISCARDED or timestep == Timestep::ABILITYCHOSEN){
            for(uint i=0; i<13; i++) printf("\b"); // deleting my progress-frame
            progress.pop_back();
        }
    }
    void enterTurn() override {
        progressStack.push_back(std::move(progress));
        progress = std::vector<int>();
    }
    void exitTurn() override {
        for(uint i = 0; i<13 * progress.size(); i++) printf("\b"); // deleting progress-frames for all states in the turn which were not exited yet
        fflush(stdout);
        progress = std::move(progressStack.back());
        progressStack.pop_back();
    }
};

void ProgressLogger::message(const char* msg) const {
    std::cout << msg << '\n';
}

void ProgressLogger::message(const char* msg, float nb) const {
    std::cout << msg << nb << '\n';
}
