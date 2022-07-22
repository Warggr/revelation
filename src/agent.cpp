#include "agent.hpp"
#include "constants.hpp"
#include <map>
#include "iostream"


class HumanAgent: public Agent {
    HumanAgent() {
        //name = input("Hi! Please enter your name: ")
    }

    character chooseCharacter(const State& state) const {
        for(uint i=0; i<NB_CHARACTERS; i++) 
            if(&state.units[myId] != NULL){
                std::cout << '[' << i << "]: " << state.units[myId]->name << '\n';
        }

        std::cout << "Enter which character to select: \n";
        int iSel = 0; std::cin >> iSel;
        return state.units[myId][iSel];
    }

    ActionOrResource getDrawAction(const State& state) const {
        std::cout << "Choose [1] draw action or [2] draw resource: ";
        int iSel; std::cin >> iSel;
        return (iSel == 1) ? ActionOrResource::ACTION : ActionOrResource::RESOURCES;
    }

    MoveDecision getMovement(const State& state) const {
        character charSel = chooseCharacter(state);
        std::vector<position> possibleMovs = state.allMovementsForCharacter(charSel);
        for(int i = 0; i<possibleMovs.size(); i++)
            std::cout << '[' << i << "]: to " << possibleMovs[i] << '\n';
        std::cout << "Enter which position to select: \n";
        uint iSel; std::cin >> iSel;
        position movSel = possibleMovs[iSel];
        return MoveDecision(charSel.pos, movSel);
    }

    ActionDecision getAction(const State& state) const override {
        ActionDecision ret(nullptr, nullptr, nullptr, position(false, 0));
        std::vector<Card> cards = getMyPlayer(state).actions
        if(cards.size() == 0)
            return nullptr;
        for(int i=0; i<cards.size(); i++){
            std::cout << '[' << (i+1) << "]: " << card << '\n';
        }
        std::cout << "Choose a card, any card (or 0 to skip):\n";
        uint iSel; std::cin >> iSel;
        if(iSel == 0)
            return nullptr;
        ret.card = cards[iSel - 1];
        if(card == ActionCard.DEFENSE){
            ret.subject = chooseCharacter(state);
        } else {
            std::map<Character*, std::vector<Character*>> allPossibleAttacks = state.allAttacks();
            if(allPossibleAttacks.size() == 0)
                return nullptr;
            std::vector<std::array<Character, 2>> array;
            int i = 1;
            for(const auto& key : allPossibleAttacks){
                Character* unit = state.aliveUnits[ myId ][ key ];
                std::cout << unit->name << '\n';
                for(const auto& enemy : allPossibleAttacks[key]){
                    startCharacter = (enemy == allPossibleAttacks[key][-1]) ? "└" : "├";
                    std::cout << '[' << i << ']' << startCharacter << "─" << enemy.name << '\n';
                    array.emplace_back( key, enemy );
                    i += 1;
                }
            }
            std::cout << "Enter which attack to select: ";
            uint iSel; std::cin >> iSel;
            ret.subject = state.aliveUnits[ myId ][ array[iSel - 1][0] ];
            ret.object = array[iSel - 1][1];
        }
        return ret;
    }
};

class SearchAgent: public Agent {
    SearchAgent(){
    };
    int evaluateStep(int myId, State oldState, Step step){
        if(step.type == "atk":
            lostHP = step.kwargs[ "lostLife" ]
            minDistance = 30
            posVictim = step.kwargs["object"]
            for myUnit in oldState.aliveUnits[ myId ]:
                if(myUnit is not None and manhattanDistance(myUnit.position, posVictim) < minDistance:
                    minDistance = manhattanDistance( myUnit.position, posVictim )
            enemy = oldState.getBoardField( posVictim )
            nbTurnsBeforeAttack = (minDistance - enemy.rng)
            if(nbTurnsBeforeAttack <= 0:
                nbTurnsBeforeAttack = 1
            danger = math.ceil( lostHP * enemy.mov / nbTurnsBeforeAttack )
            return danger
        elif(step.type == "def":
            return 50
        elif(step.type == "move":
            return -1
        else:
            return 0
    }
    def getDrawAction(const State& state){
        ret = plans[0]
        plans = plans[1:]
        return ret
    def getMovement(const State& state) -> MoveDecision:
        ret = plans[0]
        plans = plans[1:]
        return ret
    def getAbility(const State& state) -> AbilityDecision:
        ret = plans[0]
        plans = plans[1:]
        return ret
    def getAction(const State& state) -> ActionDecision:
        ret = plans[0]
        plans = plans[1:]
        return ret
    def onBegin(const State& state){
        (state, decisions, heuristic) = planAhead( myId, state, 2)
        print("Found", state, decisions, heuristic)
        plans = decisions
    def planAhead(myId : int, const State& state, maxDepth : int){
        //print("Starting minmax with nbPaths = 0")
        nbPaths = 0
        maxHeur = None
        maxHeurTemp = -20
        minTolerated = -20
        bestMoves = None
        bestState = None
        stack = [ (state, [], 0, 0) ] //state, decision history, heuristic, depth
        while stack: //not empty
            active = stack.pop()
            state = active[0]

            print("nbPaths is", nbPaths)
            if(nbPaths > 10000:
                raise Exception()
            //print("unloading step of type ", state.timestep, "@", active[3])
            //print("maxHeur is", maxHeur)
//            print('with cards:', [ len(i.actionDeck.cards) for i in state.players ])
//            print( 'History', active[1] )

            if(state.timestep == Timestep.BEGIN:
                decision = ActionOrResource.ACTION
                (newState, step) = state.stepDraw( decision )
                stack.append( (newState, active[1] + [ decision ], active[2] + evaluateStep( myId, state, step ), active[3] ) )
            elif(state.timestep == Timestep.DISCARDED or state.timestep == Timestep.MOVEDfirst:
                (newState, step) = state.stepMov( None )
                stack.append( (newState, active[1] + [ None ], active[2] + evaluateStep( myId, state, step ), active[3] ) )
                //nbChildren = 1
                for charSel in state.aliveUnits[ myId ]:
                    if(charSel is not None:
                        // print( "Examining character", charSel.cid, "at", charSel.position)
                        possibleMovs = state.allMovementsForCharacter(charSel)
                        for movSel in possibleMovs:
                            decision = MoveDecision( charSel.position, movSel )
                            (newState, step) = state.stepMov( decision )
                            stack.append( (newState, active[1] + [ decision ], active[2] + evaluateStep( myId, state, step ), active[3] ) )
                            //nbChildren += 1
                //print("Move action had", nbChildren, "children")
            elif(state.timestep == Timestep.MOVEDlast:
                decision = AbilityDecision()
                (newState, step) = state.stepAbil( decision )
                stack.append( (newState, active[1] + [ decision ], active[2] + evaluateStep( myId, state, step ), active[3] ) )
            elif(state.timestep == Timestep.ABILITYCHOSEN:
                (newState, step) = state.stepAct(None)
                stack.append( (newState, active[1] + [ None ], active[2] + evaluateStep( myId, state, step ), active[3]) ) // the "pass" option
                //print("Appending to stack "pass"", len(stack))
                cards = state.players[ myId ].actions
                ret = ActionDecision(None, None, None, position(false, 0))
                //nbChildren = 0
                for card in cards:
                    ret.card = card
                    if(card == ActionCard.DEFENSE:
                        for subject in state.aliveUnits[ myId ]:
                            if(subject is not None:
                                ret.subject = subject
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, active[1] + [ ret ], active[2] + evaluateStep( myId, state, step ), active[3]) )
                                //print("Appending defense to stack", len(stack))
                                //nbChildren += 1
                    else:
                        allPossibleAttacks = state.allAttacks()
                        for aggressor in allPossibleAttacks:
                            for victim in allPossibleAttacks[aggressor]:
                                ret.subject = state.aliveUnits[ myId ][ aggressor ]
                                ret.object = victim
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, active[1] + [ ret ], active[2] + evaluateStep( myId, state, step ), active[3]) )
                                //print("Appending attack to stack", len(stack))
                                //nbChildren += 1
                //print("Ability action has", nbChildren, "children")
            elif(state.timestep == Timestep.ACTED:
                (newState, step) = state.endTurn()
                //print(active[3], maxDepth)
                if(active[3] == maxDepth:
                    nbPaths += 1
                    //print("One path found! Heuristic:", active[2], ", max", maxHeur)
                    //print("Stacksize", len(stack))
                    if(maxHeur is None or active[2] > maxHeur:
                        //print("Best path found! Heuristic:", active[2], ", max", maxHeur)
                        bestMoves = active[1]
                        bestState = newState
                        maxHeur = active[2]
                else:
                    if(active[2] >= minTolerated:
                        if(active[2] > maxHeurTemp:
                            maxHeurTemp = active[2]
                            if(minTolerated < maxHeurTemp - 20:
                                minTolerated = maxHeurTemp - 20
                        else:
                            minTolerated += 1
                        print("MINNING")
                        //print(len(stack))
                        //print(active[2], ", tolerate", minTolerated, "-", maxHeurTemp)
                        (newState, decisions, heuristic) = planAhead( 1 - myId, newState, 0 )
                        print("RETURN TO MAXING")
                        //print(newState.timestep)
                        stack.append( ( newState, active[1], active[2] - heuristic, active[3] + 2 ) )
        print(nbPaths, "possible futures found")
        //raise Exception()
        return ( bestState, bestMoves, maxHeur )
