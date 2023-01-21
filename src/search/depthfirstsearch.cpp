#include "depthfirstsearch.hpp"

std::shared_ptr<NoOpLogger> NoOpLogger::instance = std::make_shared<NoOpLogger>();

bool DepthFirstSearch::addEndState(const State& state, const DecisionList& decisions, Heuristic::Value heurVal, const ProcessContext& pc) {
    /* This is depth-first, so we directly go deeper when we enter an end-of-turn state. */
    logger->enter(Timestep::ACTED, 1);
    SearchPolicy* subDepth = enterOpponentsTurn();
    if(!subDepth)
        return SearchPolicy::addEndState(state, decisions, heurVal, pc );

    auto [ nbTurnsMe, nbTurnsOpponent ] = subDepth->asTuple();
    if(nbTurnsMe != -1){ //asTuple returns -1 when counting the number of remaining turns does not make sense
        // iActive has already switched to the opponent
        Heuristic::Value max_bound = heurVal + heuristic.evaluateMaxForState(state.iActive, state, nbTurnsOpponent);
        Heuristic::Value min_bound = heurVal - heuristic.evaluateMaxForState(1 - state.iActive, state, nbTurnsMe - 1);
        if(min_bound >= maxHeurAllowed){ // we are in any case over the maximal allowed value
            bestState = State::invalid();
            return true;
        }
        if(max_bound <= maxHeur) // in any case, we already had a better solution
            return false;
    }

    logger->message("MINNING with cut-off at", worstOpponentsHeuristic);
    logger->enterTurn();
    subDepth->maxHeurAllowed = worstOpponentsHeuristic;
    subDepth->planAhead(state, pc);
    logger->exitTurn();
    auto [ newState, _decisions, heurDiff ] = subDepth->getResults();
    (void) _decisions;

    if (State::isInvalid(newState)) { // this happens when the search was better than bestOpponentsHeuristic and was cut off
        logger->message("Search cut off");
        return false;
    }
    if (heurDiff < worstOpponentsHeuristic) {
        worstOpponentsHeuristic = heurDiff;
    }
    heurVal -= heurDiff;
    logger->message("Search returned", heurDiff);
    return SearchPolicy::addEndState(newState, decisions, heurVal, pc);
}

void UntilSomeoneDiesDFS::init(const State& state){
    PerpetualDFS::init(state);
    nbAliveUnits = state.getNbAliveUnits();
}

bool UntilSomeoneDiesDFS::addEndState(const State& state, const DecisionList& decisions, Heuristic::Value heurVal, const ProcessContext& pc){
    if(state.getNbAliveUnits() == this->nbAliveUnits)
        return SearchPolicy::addEndState(std::move(state), decisions, heurVal, pc); //do not search further
    else
        return DepthFirstSearch::addEndState(std::move(state), decisions, heurVal, pc);
}

unsigned AdaptiveDepthFirstSearch::estimateNbBranches(){
    unsigned nbBranches = currentChildrenCount[ currentLevel ];
    for(int i = currentLevel; i != 0; i--){
        nbBranches = ( nbBranches + sumChildren[ i ] ) * ( currentChildrenCount[ i - 1 ] / (nbChildren[i] + 1) ) + 1;
    }
    return nbBranches;
}

void AdaptiveDepthFirstSearch::init(const State&){
    nodes = 1;
    currentLevel = -1;
    opponentsTurn = nullptr;
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
        opponentsTurn = std::make_unique<AdaptiveDepthFirstSearch>(heuristic, logger, maxNodes / nodes);
        return opponentsTurn.get();
    }
}

std::tuple<int, int> AdaptiveDepthFirstSearch::asTuple() {
    unsigned log = 0;
    unsigned i = maxNodes;
    while(i > 1){
        i = i / nodes - 1;
        log += 1;
    }
    return { log / 2 + log % 2, log / 2 };
}
