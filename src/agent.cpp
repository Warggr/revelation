#include "agent.hpp"
#include "constants.hpp"

class HumanAgent: public Agent {
    HumanAgent() {
        //self.name = input("Hi! Please enter your name: ")
    }

    Character* chooseCharacter(const State& state) override {
        for i, char in enumerate(state.aliveUnits[ self.myId ]):
            if char is not None:
                print(f'[{i}]: {char.name}')
        iSel = int(input('Enter which character to select: '))
        return state.aliveUnits[ self.myId ][iSel]
    }

    def getDrawAction(self, state : 'State') -> ActionOrResource:
        iSel = input('Choose [1] draw action or [2] draw resource: ')
        return ActionOrResource.ACTION if iSel == '1' else ActionOrResource.RESOURCES

    def getMovement(self, state : 'State') -> MoveDecision:
        charSel = self.chooseCharacter(state)
        possibleMovs = state.allMovementsForCharacter(charSel)
        for i, mov in enumerate(possibleMovs):
            print(f'[{i}]: to ({mov[0]}, {mov[1]})')
        iSel = int(input('Enter which position to select: '))
        movSel = possibleMovs[iSel]
        return MoveDecision(charSel.position, movSel)

    def getAction(self, state : 'State') -> ActionDecision:
        ret = ActionDecision(None, None, None)
        cards = self.getMyPlayer(state).actions
        if cards == []:
            return None
        for i, card in enumerate(cards):
            print(f'[{i + 1}]: {card}')
        iSel = int(input('Choose a card, any card (or 0 to skip): '))
        if iSel == 0:
            return None
        else:
            ret.card = cards[iSel - 1]
            if card == ActionCard.DEFENSE:
                ret.subject = self.chooseCharacter(state)
            else:
                allPossibleAttacks = state.allAttacks()
                if len(allPossibleAttacks) == 0:
                    return None
                array = []
                i = 1
                for key in allPossibleAttacks:
                    unit = state.aliveUnits[ self.myId ][ key ]
                    print(f'{ unit.name }')
                    for enemy in allPossibleAttacks[key]:
                        startCharacter = '└' if enemy == allPossibleAttacks[key][-1] else '├';
                        print(f'[{ i }]{ startCharacter }─{ enemy.name }')
                        array.append( [ key, enemy ] )
                        i += 1
                iSel = int(input('Enter which attack to select: '))
                ret.subject = state.aliveUnits[ self.myId ][ array[iSel - 1][0] ]
                ret.object = array[iSel - 1][1]
            return ret

def manhattanDistance( pos1, pos2 ):
    return abs( pos1[0] - pos2[0] ) + abs( pos1[1] - pos2[1] )

class SearchAgent(Agent):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.plans = []
    def evaluateStep(self, myId, oldState, step):
        if step.type == 'atk':
            lostHP = step.kwargs[ 'lostLife' ]
            minDistance = 30
            posVictim = step.kwargs['object']
            for myUnit in oldState.aliveUnits[ myId ]:
                if myUnit is not None and manhattanDistance(myUnit.position, posVictim) < minDistance:
                    minDistance = manhattanDistance( myUnit.position, posVictim )
            enemy = oldState.getBoardField( posVictim )
            nbTurnsBeforeAttack = (minDistance - enemy.rng)
            if nbTurnsBeforeAttack <= 0:
                nbTurnsBeforeAttack = 1
            danger = math.ceil( lostHP * enemy.mov / nbTurnsBeforeAttack )
            return danger
        elif step.type == 'def':
            return 50
        elif step.type == 'move':
            return -1
        else:
            return 0
    def getDrawAction(self, state : 'State'):
        ret = self.plans[0]
        self.plans = self.plans[1:]
        return ret
    def getMovement(self, state : 'State') -> MoveDecision:
        ret = self.plans[0]
        self.plans = self.plans[1:]
        return ret
    def getAbility(self, state : 'State') -> AbilityDecision:
        ret = self.plans[0]
        self.plans = self.plans[1:]
        return ret
    def getAction(self, state : 'State') -> ActionDecision:
        ret = self.plans[0]
        self.plans = self.plans[1:]
        return ret
    def onBegin(self, state : 'State'):
        (state, decisions, heuristic) = self.planAhead( self.myId, state, 2)
        print('Found', state, decisions, heuristic)
        self.plans = decisions
    def planAhead(self, myId : int, state : 'State', maxDepth : int):
        #print('Starting minmax with nbPaths = 0')
        nbPaths = 0
        maxHeur = None
        maxHeurTemp = -20
        minTolerated = -20
        bestMoves = None
        bestState = None
        stack = [ (state, [], 0, 0) ] #state, decision history, heuristic, depth
        while stack: #not empty
            active = stack.pop()
            state = active[0]

            print('nbPaths is', nbPaths)
            if nbPaths > 10000:
                raise Exception()
            #print("unloading step of type ", state.timestep, '@', active[3])
            #print('maxHeur is', maxHeur)
#            print('with cards:', [ len(i.actionDeck.cards) for i in state.players ])
#            print( 'History', active[1] )

            if state.timestep == Timestep.BEGIN:
                decision = ActionOrResource.ACTION
                (newState, step) = state.stepDraw( decision )
                stack.append( (newState, active[1] + [ decision ], active[2] + self.evaluateStep( myId, state, step ), active[3] ) )
            elif state.timestep == Timestep.DISCARDED or state.timestep == Timestep.MOVEDfirst:
                (newState, step) = state.stepMov( None )
                stack.append( (newState, active[1] + [ None ], active[2] + self.evaluateStep( myId, state, step ), active[3] ) )
                #nbChildren = 1
                for charSel in state.aliveUnits[ myId ]:
                    if charSel is not None:
                        # print( 'Examining character', charSel.cid, 'at', charSel.position)
                        possibleMovs = state.allMovementsForCharacter(charSel)
                        for movSel in possibleMovs:
                            decision = MoveDecision( charSel.position, movSel )
                            (newState, step) = state.stepMov( decision )
                            stack.append( (newState, active[1] + [ decision ], active[2] + self.evaluateStep( myId, state, step ), active[3] ) )
                            #nbChildren += 1
                #print('Move action had', nbChildren, 'children')
            elif state.timestep == Timestep.MOVEDlast:
                decision = AbilityDecision()
                (newState, step) = state.stepAbil( decision )
                stack.append( (newState, active[1] + [ decision ], active[2] + self.evaluateStep( myId, state, step ), active[3] ) )
            elif state.timestep == Timestep.ABILITYCHOSEN:
                (newState, step) = state.stepAct(None)
                stack.append( (newState, active[1] + [ None ], active[2] + self.evaluateStep( myId, state, step ), active[3]) ) # the "pass" option
                #print('Appending to stack "pass"', len(stack))
                cards = state.players[ myId ].actions
                ret = ActionDecision(None, None, None)
                #nbChildren = 0
                for card in cards:
                    ret.card = card
                    if card == ActionCard.DEFENSE:
                        for subject in state.aliveUnits[ myId ]:
                            if subject is not None:
                                ret.subject = subject
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, active[1] + [ ret ], active[2] + self.evaluateStep( myId, state, step ), active[3]) )
                                #print('Appending defense to stack', len(stack))
                                #nbChildren += 1
                    else:
                        allPossibleAttacks = state.allAttacks()
                        for aggressor in allPossibleAttacks:
                            for victim in allPossibleAttacks[aggressor]:
                                ret.subject = state.aliveUnits[ myId ][ aggressor ]
                                ret.object = victim
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, active[1] + [ ret ], active[2] + self.evaluateStep( myId, state, step ), active[3]) )
                                #print('Appending attack to stack', len(stack))
                                #nbChildren += 1
                #print('Ability action has', nbChildren, 'children')
            elif state.timestep == Timestep.ACTED:
                (newState, step) = state.endTurn()
                #print(active[3], maxDepth)
                if active[3] == maxDepth:
                    nbPaths += 1
                    #print('One path found! Heuristic:', active[2], ', max', maxHeur)
                    #print('Stacksize', len(stack))
                    if maxHeur is None or active[2] > maxHeur:
                        #print('Best path found! Heuristic:', active[2], ', max', maxHeur)
                        bestMoves = active[1]
                        bestState = newState
                        maxHeur = active[2]
                else:
                    if active[2] >= minTolerated:
                        if active[2] > maxHeurTemp:
                            maxHeurTemp = active[2]
                            if minTolerated < maxHeurTemp - 20:
                                minTolerated = maxHeurTemp - 20
                        else:
                            minTolerated += 1
                        print('MINNING')
                        #print(len(stack))
                        #print(active[2], ', tolerate', minTolerated, '-', maxHeurTemp)
                        (newState, decisions, heuristic) = self.planAhead( 1 - myId, newState, 0 )
                        print('RETURN TO MAXING')
                        #print(newState.timestep)
                        stack.append( ( newState, active[1], active[2] - heuristic, active[3] + 2 ) )
        print(nbPaths, 'possible futures found')
        #raise Exception()
        return ( bestState, bestMoves, maxHeur )
