#include "depthfirstsearch.hpp"
#include <iostream>

bool DepthFirstSearch::addEndState(State state, const DecisionList& decisions, Heuristic::Value heurVal) {
    /* This is depth-first, so we directly go deeper when we enter an end-of-turn state. */
    logger.enter(Timestep::ACTED, 1);
    SearchPolicy* subDepth = enterOpponentsTurn();
    if(subDepth) {
        auto [ nbTurnsMe, nbTurnsOpponent ] = subDepth->asTuple();
        // iActive has already switched to the opponent
        Heuristic::Value max_bound = heurVal + heuristic.evaluateMaxForState(state.iActive, state, nbTurnsOpponent);
        Heuristic::Value min_bound = heurVal - heuristic.evaluateMaxForState(1 - state.iActive, state, nbTurnsMe - 1);
        if(min_bound >= maxHeurAllowed){ // we are in any case over the maximal allowed value
            bestState = State::invalid();
            return true;
        }
        if(max_bound <= maxHeur) // in any case, we already had a better solution
            return false;

        logger.message("MINNING with cut-off at", worstOpponentsHeuristic);
        logger.enterTurn();
        subDepth->maxHeurAllowed = worstOpponentsHeuristic;
        subDepth->planAhead(state);
        logger.exitTurn();
        auto [ newState, _decisions, heurDiff ] = subDepth->getResults();
        (void) _decisions;
        state = newState;

        if (State::isInvalid(state)) { // this happens when the search was better than bestOpponentsHeuristic and was cut off
            logger.message("Search cut off");
            return false;
        }
        if (heurDiff < worstOpponentsHeuristic) {
            worstOpponentsHeuristic = heurDiff;
        }
        heurVal -= heurDiff;
        logger.message("Search returned", heurDiff);
    }
    return SearchPolicy::addEndState(state, decisions, heurVal);
}

unsigned AdaptiveDepthFirstSearch::estimateNbBranches(){
    unsigned nbBranches = currentChildrenCount[ currentLevel ];
    for(int i = currentLevel; i != 0; i--){
        nbBranches = ( nbBranches + sumChildren[ i ] ) * ( currentChildrenCount[ i - 1 ] / (nbChildren[i] + 1) ) + 1;
    }
    return nbBranches;
}

void AdaptiveDepthFirstSearch::informNbChildren(unsigned nbChildren, Timestep timestep) {
    DepthFirstSearch::informNbChildren(nbChildren, timestep);
    int timestepLevel = usedLevelsMap[ timestep ];
    //std::cout << timestep << "->" << usedLevelsMap[ timestep ] << "->" << nbChildren << '\n';
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

SearchPolicy* AdaptiveDepthFirstSearch::enterOpponentsTurn() {
    nodes = estimateNbBranches();
    if(maxNodes <= 1.5 * (nodes * nodes)) return nullptr;
    else{
        if(opponentsTurn) delete opponentsTurn;
        opponentsTurn = new AdaptiveDepthFirstSearch(heuristic, logger, maxNodes / nodes);
        return opponentsTurn;
    }
}

std::tuple<unsigned, unsigned> AdaptiveDepthFirstSearch::asTuple() {
    unsigned log = 0;
    unsigned i = maxNodes;
    while(i > 1){
        i = i / nodes - 1;
        log += 1;
    }
    return { log / 2 + log % 2, log / 2 };
}

void SimpleIndentLogger::message( const char* msg ) const {
    for(unsigned i = 0; i < indent; i++) std::cout << "    ";
    std::cout << msg << '\n';
}
void SimpleIndentLogger::message( const char* msg, float x ) const {
    for(unsigned i = 0; i < indent; i++) std::cout << "    ";
    std::cout << msg << x << '\n';
}

void ProgressBar::enter(Timestep timestep, unsigned nbChildren) {
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

void ProgressBar::exit(Timestep timestep) {
    if(timestep == Timestep::DISCARDED or timestep == Timestep::ABILITYCHOSEN){
        for(uint i=0; i<13; i++) printf("\b"); // deleting my progress-frame
        progress.pop_back();
    }
}

void ProgressBar::enterTurn() {
    progressStack.push_back(std::move(progress));
    progress = std::vector<int>();
}

void ProgressBar::exitTurn() {
    for(uint i = 0; i<13 * progress.size(); i++) printf("\b"); // deleting progress-frames for all states in the turn which were not exited yet
    fflush(stdout);
    progress = std::move(progressStack.back());
    progressStack.pop_back();
}

void ProgressLogger::message(const char* msg) const {
    std::cout << msg << '\n';
}

void ProgressLogger::message(const char* msg, float nb) const {
    std::cout << msg << nb << '\n';
}
