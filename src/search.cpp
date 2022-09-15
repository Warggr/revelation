#include "search.hpp"
#include "state.hpp"
#include "agent.hpp"
#include "position.hpp"
#include "search/heuristic.hpp"
#include <tuple>
#include <unordered_set>
#include <cstdio>
#include <cassert>
#include <ostream>

std::ostream& operator<<(std::ostream& o, const position& pos);

DiscardDecision SearchAgent::getDiscard(const State& state) {
    assert(state == plans.beforeDiscard);
    (void)state;
    return plans.discard;
}

MoveDecision SearchAgent::getMovement(const State& state, unsigned nb) {
    assert(state == plans.beforeMove[nb]);
    (void)state;
    return plans.moves[nb];
}

AbilityDecision SearchAgent::getAbility(const State& state) {
    assert(state == plans.beforeAbility);
    (void)state;
    return plans.ability;
}

ActionDecision SearchAgent::getAction(const State& state) {
    assert(state == plans.beforeAction);
    (void)state;
    return plans.action;
}

void SearchAgent::onBegin(const State& state) {
    currentSpecialAction = 0;
    searchPolicy.planAhead(state);
    State newState; Heuristic::Value heurVal;
    std::tie(newState, plans, heurVal) = searchPolicy.getResults();
}

std::ostream& operator<<(std::ostream& o, const Board& board){
    o << "---BOARD---\n";
    for(uint row = 0; row < 2; row++){
        for(uint col = 0; col < FULL_BOARD_WIDTH; col++){
            const BoardTile& tile = board[row][col];
            if(BoardTile::isEmpty(tile)) o << "[   ]";
            else o << '[' << static_cast<int>(tile.team) << '.' << tile.index << ']';
        }
        o << '\n';
    }
    return o;
}

void SearchPolicy::planAhead(const State& startState){
    /*
    Constructs all descendants of @param state until the end of turn.
    The order in which states are processed depend on the container.
    All end-of-turn states are then given to the callback function this->addEndState().
    For depth-first search with multiple turns, addEndState recursively calls planAhead again.
    */

    //Reset this
    maxHeur = std::numeric_limits<float>::min();
    worstOpponentsHeuristic = std::numeric_limits<float>::max();

    init(startState);
    Container<SearchNode>& container = getContainer();
    container.addChild( SearchNode(startState, DecisionList(), 0) );

    while(container.hasChildren()){
        SearchNode node = container.popChild();

        const State& state = node.state;
        if(state.getWinner() != 0){
            bool finishEarly = addWinState(state, node.decisions);
            if(finishEarly) return;
        } else if(state.timestep != Timestep::ACTED){
            unsigned nbChildren = pushChildStates(node, container, heuristic);
            informNbChildren(nbChildren, state.timestep);
        } else {
            auto [ newState, step ] = state.beginTurn();
            bool finishEarly = addEndState(newState, node.decisions, node.heurVal);
            if(finishEarly) return;
        }
    }
    this->exit();
}

using HashKey = int;

static_assert ( FULL_BOARD_WIDTH < (1 << 4) );

unsigned char hashPosition( const position& pos ){ //max. 4 bits for column and 1 for row
    return 2 * pos.column + pos.column;
}

HashKey hashBoard( const UnitList& units ){
    int retVal = 0;
    for(unsigned i = 0; i<6; i++)
        retVal += (isDead(units[i]) ? 0 : hashPosition(units[i]->pos) + 1) << (5*i);
    return retVal;
}

unsigned pushChildStates(const SearchNode& stackFrame, Container<SearchNode>& putBack, const Heuristic& heuristic){
    /*
    Creates all children states of state (the first element in the @param stackFrame) and adds them into the data structure @param putBack.
    @returns the number of children inserted this way.
    */
    const State& state = stackFrame.state;
    state.checkConsistency();
    switch(state.timestep){
    case Timestep::BEGIN:{
        auto [ newState, step ] = state.stepDraw( );
        SearchNode newFrame = stackFrame.copy(newState, heuristic.evaluateStep( state.iActive, state, *step ));
        putBack.addChild( newFrame );
        return 1;
    }

    case Timestep::DREW: {
        for(unsigned i = 0; i < state.players[state.iActive].getActions().size(); i++){
            DiscardDecision decision(i);
            auto [ newState, step ] = state.stepDiscard(decision);
            SearchNode newFrame = stackFrame.copy(newState, heuristic.evaluateStep( state.iActive, state, *step ));
            newFrame.decisions.discard = decision;
            #ifndef NDEBUG
                newFrame.decisions.beforeDiscard = state;
            #endif
            putBack.addChild( newFrame );
        }
        return state.players[state.iActive].getActions().size();
    }

    case Timestep::DISCARDED:{
        std::unordered_set<HashKey> allPossibleMoves;
        //nbChildrenFiltered = 0

        struct ExtendedMoveDecision {
            MoveDecision dec;
            #ifndef NDEBUG
                State stateBefore;
            #endif
        };

        using SubStackFrame = std::tuple<State, std::array<ExtendedMoveDecision, 2>, Heuristic::Value>;

        std::vector<SubStackFrame> oldStates;
        oldStates.emplace_back( state, std::array<ExtendedMoveDecision, 2>(), stackFrame.heurVal );
        for(unsigned iMoveRound = 0; iMoveRound<=2; iMoveRound++){
            std::vector<SubStackFrame> newStates;
            for( const auto& [ subState, decisions, heurValOld ] : oldStates ){
                /* add all oldStates to putBack (for round 0, 1, and 2) */
                Heuristic::Value heurVal = heurValOld;
                HashKey stateid = hashBoard(subState.units[subState.iActive]);
                //print("with", [ printDecision(decision) if decision else None for decision in decisions ], ", stateid would be ", stateid);
                if( allPossibleMoves.find(stateid) != allPossibleMoves.end() ){ //already in allPossibleMoves
                    //nbChildrenFiltered += 1
                    continue;
                }
                auto newStackFrame = stackFrame.copy(subState, 0);
                for(unsigned iPreviousDecision = 0; iPreviousDecision<iMoveRound; iPreviousDecision++){
                    newStackFrame.decisions.moves[iPreviousDecision] = decisions[iPreviousDecision].dec;
                    #ifndef NDEBUG
                        newStackFrame.decisions.beforeMove[iPreviousDecision] = decisions[iPreviousDecision].stateBefore;
                    #endif
                }
                if(iMoveRound != 2){
                    //add an additional "pass" decision to mark the end of movements
                    MoveDecision decision = MoveDecision::pass();
                    auto [ newSubState, step ] = subState.stepMov(decision);
                    assert(step->isPass());
                    newStackFrame = newStackFrame.copy( newSubState, heuristic.evaluateMoveStep( state.iActive, newSubState, *step ) );
                    newStackFrame.decisions.moves[iMoveRound] = decision;
                    #ifndef NDEBUG
                        newStackFrame.decisions.beforeMove[iMoveRound] = subState;
                    #endif
                }
                putBack.addChild( newStackFrame );
                allPossibleMoves.insert( stateid );

                /* generate all newStates (round 0 generates 1, 1 generates 2, and 2 is skipped) */
                if(iMoveRound != 2){
                    for(const auto& charSel : subState.units[ subState.iActive ]){
                        if(not isDead(charSel)){
                            auto possibleMovs = subState.allMovementsForCharacter(*charSel);
                            for(const auto& movSel : possibleMovs){
                                std::array<ExtendedMoveDecision, 2> newDecisions = decisions;
                                newDecisions[iMoveRound].dec = movSel;
                                #ifndef NDEBUG
                                    newDecisions[iMoveRound].stateBefore = subState;
                                #endif
                                auto [ newState, step ] = subState.stepMov( newDecisions[iMoveRound].dec );
                                heurVal += heuristic.evaluateMoveStep( state.iActive, newState, *step );

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
        #ifndef NDEBUG
            retVal.decisions.beforeAbility = state;
        #endif
        putBack.addChild( retVal );
        return 1;
    }

    case Timestep::ABILITYCHOSEN:{
        { // the "pass" decision
            ActionDecision passDecision = ActionDecision::pass();
            auto [ newState, step ] = state.stepAct(passDecision);
            SearchNode newFrame = stackFrame.copy(newState, heuristic.evaluateActionStep( state.iActive, state, *step ));
            newFrame.decisions.action = passDecision;
            #ifndef NDEBUG
                newFrame.decisions.beforeAction = state;
            #endif
            putBack.addChild( newFrame );
        }
        ActionDecision decision;
        unsigned nbChildren = 1; // already one child: the "pass" decision

        for(auto card : state.players[ state.iActive ].getActions()){
            decision.card = card;
            if(card == ActionCard::DEFENSE){
                for(const auto& subject : state.units[ state.iActive ]){
                    if(not isDead(subject)){
                        decision.subjectPos = subject->pos;
                        auto [ newState, step ] = state.stepAct( decision );
                        SearchNode retVal = stackFrame.copy( newState, heuristic.evaluateActionStep( state.iActive, state, *step ) );
                        retVal.decisions.action = decision;
                        #ifndef NDEBUG
                            retVal.decisions.beforeAction = state;
                        #endif
                        putBack.addChild( retVal );
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
                        SearchNode retVal = stackFrame.copy( newState, heuristic.evaluateActionStep( state.iActive, state, *step ) );
                        retVal.decisions.action = decision;
                        #ifndef NDEBUG
                            retVal.decisions.beforeAction = state;
                        #endif
                        putBack.addChild( retVal );
                        nbChildren += 1;
                    }
                }
            }
        }
        return nbChildren;
    }

    default:
        assert(false); return 0; //Timestep::ACTED (aka end of turns) are supposed to be handled differently
    }
}
