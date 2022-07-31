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

unsigned pushChildStates(const StackFrame& stackFrame, SearchPolicy& putBack, const Heuristic& heuristic){
    /*
    Creates all children states of state (the first element in the @param stackFrame) and adds them into the data structure @param putBack.
    @returns the number of children inserted this way.
    */
    const State& state = stackFrame.state;
    switch(state.timestep){
    case Timestep::BEGIN:{
        auto decision = ActionOrResource::ACTION;
        auto [ newState, step ] = state.stepDraw( decision );
        StackFrame newFrame = stackFrame.copy(newState, heuristic.evaluateStep( state.iActive, state, step ));
        newFrame.decisions.draw = decision;
        putBack.addChild( newFrame );
        return 1;
    }

    case Timestep::DISCARDED:{
        std::unordered_set<HashKey> allPossibleMoves;
        //nbChildrenFiltered = 0

        using SubStackFrame = std::tuple<State, std::array<MoveDecision, 2>, Heuristic::Value>;

        std::vector<SubStackFrame> oldStates;
        oldStates.push_back( std::make_tuple( state, std::array<MoveDecision, 2>(), stackFrame.heurVal ) );
        for(unsigned i = 0; i<3; i++){
            std::vector<SubStackFrame> newStates;
            for( auto tuple : oldStates ){
                const auto& [ subState, decisions, heurVal ] = tuple;
                auto stateid = hashBoard(subState.units[subState.iActive]);
                //print("with", [ printDecision(decision) if decision else None for decision in decisions ], ", stateid would be ", stateid);
                if( allPossibleMoves.find(stateid) != allPossibleMoves.end() ){ //already in allPossibleMoves
                    //nbChildrenFiltered += 1
                    continue;
                }
                auto newStackFrame = stackFrame.copy(subState, 0);
                if(i != 2){
                    MoveDecision decision = MoveDecision::pass();
                    auto [ newSubState, step ] = subState.stepMov(decision);
                    assert(isPass(step));
                    newStackFrame = newStackFrame.copy( newSubState, heuristic.evaluateStep( state.iActive, newSubState, step ) );
                    newStackFrame.decisions.moves[i+1] = decision;
                }
                putBack.addChild( newStackFrame );
                allPossibleMoves[ stateid ] = true;

                if(i != 2){
                    for(auto charSel : subState.units[ subState.iActive ]){
                        if(not isDead(charSel)){
                            auto possibleMovs = subState.allMovementsForCharacter(charSel);
                            for(auto movSel : possibleMovs){
                                std::array<MoveDecision, 2> newDecisions = decisions;
                                newDecisions[i+1] = MoveDecision( charSel.pos, movSel[0], movSel[1] );
                                auto [ newState, step ] = subState.stepMov( newDecisions[i+1] );
                                heurVal += heuristic.evaluateStep( step );

                                newStates.push_back( std::make_tuple(newState, newDecisions, heurVal ) );
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
        StackFrame retVal = stackFrame.copy( newState, heuristic.evaluateStep( state.iActive, state, step ) );
        retVal.decisions.ability = decision;
        putBack.addChild( retVal );
        return 1;
    }

    case Timestep::ABILITYCHOSEN:{
        { // the "pass" decision
            ActionDecision passDecision = ActionDecision::pass();
            auto [ newState, step ] = state.stepAct(passDecision);
            StackFrame newFrame = stackFrame.copy(newState, heuristic.evaluateStep( state.iActive, state, step ));
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
                        decision.subjectPos = subject.pos;
                        decision.object = nullptr;
                        auto [ newState, step ] = state.stepAct( decision );
                        StackFrame retVal = stackFrame.copy( newState, heuristic.evaluateStep( state.iActive, state, step ) );
                        retVal.ability = decision;
                        putBack.addChild( retVal );
                        //print("Appending defense to stack", len(stack));
                        nbChildren += 1;
                    }
                }
            } else {
                auto allPossibleAttacks = state.allAttacks();
                for(auto aggressor : allPossibleAttacks){
                    for(auto victim : allPossibleAttacks[aggressor]){
                        decision.subjectPos = state.units[ state.iActive ][ aggressor ].pos;
                        decision.objectPos = state.units[ 1 - state.iActive ][ victim ].pos;
                        auto [ newState, step ] = state.stepAct( decision );
                        StackFrame retVal = stackFrame.copy( newState, heuristic.evaluateStep( state.iActive, state, step ) );
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
        throw std::exception("Timestep::ACTED (aka end of turns) are supposed to be handled differently");
    }
}

class ProgressLogger{
public:
    virtual void enterTurn() = 0;
    virtual void exitTurn() = 0;
    virtual void enter(Timestep timestep, unsigned nbChildren) = 0;
    virtual void exit(Timestep timestep) = 0;
    virtual void message(const char* msg) const {
        std::cout << msg << '\n';
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
};

class ProgressBar : public ProgressLogger{
    std::vector<std::vector<int>> progressStack;
    std::vector<int> progress;
public:
    void message( const char* ) const override {};
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
            for(int i=0; i<13; i++) printf("\b"); // deleting my progress-frame
            progress.pop_back();
        }
    }
    void enterTurn() override {
        progressStack.push_back(std::move(progress));
        progress = std::vector<int>();
    }
    void exitTurn() override {
        for(int i = 0; i<13 * progress.size(); i++) printf("\b"); // deleting progress-frames for all states in the turn which were not exited yet
        fflush(stdout);
        progress = std::move(progressStack.back());
        progressStack.pop_back();
    }
};

class SearchAgent: public Agent {
    /*
    Search up to a fixed depth described by a set of parenthesis.
        - [] indicates that only my own turn will be considered
        - [ X ] my own turn will be considered, then the opponent will choose their best state with maxDepth=X
        - [ X, Y ] I will simulate my own turn, then the opponent will simulate their turn with maxDepth=X and return the result, then I will simulate the consequences of their turn with maxDepth=Y.

        For example, for a full-2-turns-ahead simulation, use maxDepth = [ [ [ [] ] ] ]
    */
    SearchAgent* opponentsTurn = nullptr;
    SearchAgent* myTurn = nullptr;

    DecisionList plans;
public:
    ~SearchAgent() { if(opponentsTurn) delete opponentsTurn; if(myTurn) delete myTurn; }
    ActionOrResource getDrawAction(const State& state) override { return plans.draw; }
    MoveDecision getMovement1(const State& state) override { return plans.moves[0]; }
    MoveDecision getMovement2(const State& state) override { return plans.moves[1]; }
    AbilityDecision getAbility(const State& state) override { return plans.ability; }
    ActionDecision getAction(const State& state) override { return plans.action; }

    virtual void addChild(const StackFrame& child) = 0;
    virtual bool hasChildren() const = 0;

    template<typename SearchAgentType>
    SearchAgentType* addOpponent(){
        opponentsTurn = new SearchAgentType();
        return opponentsTurn;
    }
    template<typename SearchAgentType>
    SearchAgentType* addMe(){
        myTurn = new SearchAgentType();
        return myTurn;
    }
    SearchAgent* getOpponent() const { return opponentsTurn; }
    SearchAgent* getMe() const { return myTurn; }

    SearchAgent* enterOpponentsTurn() { return opponentsTurn; }
    SearchAgent* enterOwnTurn() { return myTurn; }

    std::tuple<unsigned, unsigned> asTuple() {
        unsigned myTurns = 1, opponentsTurns = 0;
        if(opponentsTurn){
            unsigned opp, me;
            std::tie(opp, me) = opponentsTurns->asTuple();
            myTurns += me;
            opponentsTurns += opp;
            if(myTurn){
                std::tie(me, opp) = myTurn->asTuple();
                myTurns += me;
                opponentsTurns += opp;
            }
        }
        return std::make_tuple( myTurns, opponentsTurns );
    }
};

class GreedyBestFirstSearchAgent: public SearchAgent{
    struct MyQueueFrame{
        StackFrame stackFrame;
        unsigned depth;

        MyQueueFrame( StackFrame frame, unsigned depth ): stackFrame(frame), depth(depth) {};
        bool operator<(const MyQueueFrame& other) const { return stackFrame.heurVal < other.stackFrame.heurVal; }
    };

    std::priority_queue<MyQueueFrame> queue;

    void addChild(const StackFrame& child) override {
        queue.push( MyQueueFrame( child, 0 ) ); //TODO depth
    }
    bool hasChildren() const override { return not queue.empty(); }
};

class DepthFirstSearchAgent: public SearchAgent {
    std::vector<StackFrame> stack;
public:
    void addChild(const StackFrame& child) override { stack.push_back(child); }
    bool hasChildren() const override { return not stack.empty(); }

    std::tuple<State, Heuristic::Value> planAhead(const State& state, const DepthPolicy& depthPolicy, const ProgressLogger& logger, Heuristic::Value maxHeurAllowed = std::numeric_limits<float>().max()){
        /*
        Constructs all descendants of @param state. up to a level defined by @param depthPolicy.
        Selects the one that is best for the active player (defined by state.iActive);
        Returns a tuple (state, decisions, heuristic) with the bext future state, the decisions that led to it, and the heuristic of that state.

        Specify @param maxHeurAllowed if(you want the function to return directly if(it reaches a certain heuristic.
        */
        logger.enterTurn();
        //nbPaths = 0
        DecisionList bestMoves;
        State bestState;
        Heuristic::Value maxHeur = std::numeric_limits<float>().min(), worstOpponentsHeuristic = std::numeric_limits<float>().max();
        depthPolicy.addChild( StackFrame(state, DecisionList(), 0) );
        while(depthPolicy.hasChildren()){
            StackFrame frame = stack.back(); stack.pop_back();
            if(frame.pass2){
                logger.exit(state.timestep);
                continue;
            } else {
                frame.pass2 = true;
                stack.push_back(frame);
            }

            const State& state = frame.state;
            if(state.timestep != Timestep::ACTED){
                nbChildren = pushChildStates(frame, stack, stateCache);
                logger.enter(state.timestep, nbChildren);
                depthPolicy.informNbChildren(nbChildren, state.timestep);
            } else {
                (newState, step) = state.beginTurn();
                logger.enter(state.timestep, 1);

                DepthPolicy* subDepth = depthPolicy.enterOpponentsTurn();
                if(subDepth != nullptr){
                    auto [ nbTurnsMe, nbTurnsOpponent ] = subDepth->asTuple();
                    // we"re using state.iActive and not newState.iActive because in newState, iActive has already switched to the opponent
                    Heuristic::Value max_bound = frame.heurVal + Heuristic.evaluateMaxForState(newState, 1 - state.iActive, nbTurnsOpponent);
                    Heuristic::Value min_bound = frame.heurVal - Heuristic.evaluateMaxForState(newState, state.iActive, nbTurnsMe);
                    if(min_bound >= maxHeurAllowed){ // we are in any case over the maximal allowed value
                        progressLogger.exitTurn();
                        return std::make_tuple(State::invalid(), min_bound);
                    }
                    if(max_bound <= maxHeur) // in any case, we already had a better solution
                        continue;

                    progressLogger.message("MINNING with cut-off at", worstOpponentsHeuristic);
                    auto [ newState, opponentsHeuristic ] = planAhead( newState, subDepth, worstOpponentsHeuristic, progressLogger );
                    if(State::isInvalid(newState)){ // this happens when the search was better than bestOpponentsHeuristic and was cut off
                        progressLogger.message("Search cut off");
                        continue;
                    }
                    if(opponentsHeuristic < worstOpponentsHeuristic){
                        worstOpponentsHeuristic = opponentsHeuristic;
                    }
                    frame.heurVal -= opponentsHeuristic;
                    progressLogger.message("Search returned", opponentsHeuristic);

                    subsubDepth = depthPolicy.enterOwnTurn();
                    if(subsubDepth != nullptr){
                        (newState, _decisions, heuristic) = planAhead( newState, subsubDepth, None, progressLogger );
                        frame.heurVal += heuristic
                    }
                }
                //nbPaths += 1

                if(frame.heurVal > maxHeur){
                    bestMoves = stackFrame.decisions;
                    bestState = newState;
                    maxHeur = frame.heurVal;
                }
            }
        }
        //print(nbPaths, "possible futures found");
        progressLogger.exitTurn();
        return std::make_tuple( bestState, bestMoves, maxHeur );
    }
};

class AdaptiveDepthPolicy : public DepthPolicy{
    constexpr static int usedLevelsMap[] = { 0, -1, 1, -1, 2, 3 };
    constexpr static int nbUsedLevels = 4;

    unsigned maxNodes;
    unsigned nodes;
    int currentLevel;
    unsigned sumChildren[nbUsedLevels]{0};
    unsigned nbChildren[nbUsedLevels]{0};
    unsigned currentChildrenCount[nbUsedLevels]{0};
public:
    AdaptiveDepthPolicy(unsigned maxNodes = 1000000): maxNodes(maxNodes), nodes(1), currentLevel(-1) {}
    void informNbChildren(unsigned nbChildren, Timestep timestep){
        // print(timestepLevel.value, "->", AdaptiveDepthPolicy.usedLevelsMap[ timestepLevel.value ], "->", nbChildren);
        timestepLevel = AdaptiveDepthPolicy.usedLevelsMap[ timestepLevel ];
        if(timestepLevel == currentLevel + 1){
            //pass
        } else if(timestepLevel == currentLevel){
            sumChildren[ timestepLevel ] += currentChildrenCount[ timestepLevel ];
            nbChildren[ timestepLevel ] += 1;
        } else if(timestepLevel == currentLevel - 1){
            sumChildren[ timestepLevel ] += sumChildren[ currentLevel ] + currentChildrenCount[ currentLevel ] + 1;
            nbChildren[ timestepLevel ] += 1;
            sumChildren[ currentLevel ] = 0;
            nbChildren[ currentLevel ] = 0;
        } else {
            assert(false);
        }
        currentLevel = timestepLevel;
        currentChildrenCount[ timestepLevel ] = nbChildren;
    }
    unsigned estimateNbBranches(){
        // print("currentChildrenCount:", currentChildrenCount);
        // print("sumChildren:", sumChildren);
        // print("nbChildren:", nbChildren);
        // print("Current level:", currentLevel);
        unsigned nbBranches = currentChildrenCount[ currentLevel ];
        for(int i = currentLevel; i != 0; i--){
            // print(f"( {nbBranches} + {sumChildren[ i ]} ) * ( {currentChildrenCount[ i - 1 ]} / ({nbChildren[i]} + 1) )", end=");
            nbBranches = ( nbBranches + sumChildren[ i ] ) * ( currentChildrenCount[ i - 1 ] / (nbChildren[i] + 1) ) + 1;
            // print(f" =: {nbBranches}");
        }
        return nbBranches;
    }
    void enterOpponentsTurn() override {
        //print("To enter or not to enter, that is the question...");
        nodes = estimateNbBranches();
        // print("MaxNodes", maxNodes, "at this depth there were", nodes);
        if(maxNodes <= 1.5 * (nodes ** 2)) return nullptr;
        else return AdaptiveDepthPolicy(maxNodes / nodes);
    }
    std::tuple<unsigned, unsigned> asTuple() override {
        log = 0;
        i = maxNodes;
        while(i > 1){
            i = i / nodes - 1;
            log += 1;
        }
        return ( log / 2 + log % 2, log / 2 );
    }
};

class PowerTimesToughnessHeuristic : public Heuristic {
    Value evaluateMaxForState(const State& state, short playerId, unsigned short nbTurnsRemaining) const override {
        myMaxAtk = 0;
        for(auto unit : state.units[ playerId ]){ //units are sorted by max atk, so the first in the list is the one with the highest atk
            if(not isDead(unit)){
                myMaxAtk = unit.maxAtk;
                break;
            }
        }
        nbMaxDamage = myMaxAtk * nbTurnsRemaining;
        heurVal = 0;
        for(auto enemy : state.units[ 1 - playerId ]){
            if(not isDead(unit)){
                if(unit.HP < nbMaxDamage){
                    heurVal += unit.maxAtk * ( unit.HP + unit.maxHP );
                    nbMaxDamage -= unit.HP
                } else {
                    heurVal += unit.maxAtk * nbMaxDamage;
                    break;
                }
            }
        }
        return heurVal;
    }
    Value evaluateStep( int myId, const State& oldState, const Step& step) const override {
        if(step.type == "atk"){
            obj = oldState.getBoardFieldDeref( step.kwargs["object"] );
            ret = step.lostLife * obj.maxAtk;
            if("delete" in step.kwargs){
                ret += obj.maxAtk * obj.maxHP
            }
            return ret;
        }
        else if(step.type == "def")
            return 50 * oldState.getBoardFieldDeref( step.kwargs["subject"] ).maxAtk;
        else if(step.type == "move")
            return -1;
        else
            return 0;
    }
};
