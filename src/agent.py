from typing import Union
from enum import Enum, unique
from constants import MAX_RESOURCES, MAX_ACTIONS, Timestep, Direction, FULL_BOARD_WIDTH
from card import ActionCard
from character import Character

import math

@unique
class ActionOrResource(Enum):
    ACTION = True
    RESOURCES = False

class MoveDecision:
    def __init__(self, frm, moves, to):
        assert isinstance(frm, tuple) and isinstance(to, tuple)
        self.frm = frm # we save the position, not the character, because the character might get invalidated
        self.moves = moves
        self.to = to

class ActionDecision:
    def __init__(self, card : ActionCard, subjectPos, objectPos = None ):
        self.card = card
        self.subjectPos = subjectPos
        self.objectPos = objectPos
    def copy(self):
        return ActionDecision(self.card, self.subjectPos, self.objectPos)

class AbilityDecision:
    def __init__(self):
        self.type = 'pass'

class Agent:
    """
    class Agent represents a decision-maker. It is an abstract class that can be implemented by an UI that interfaces with a human, or by an AI.
    """
    def __init__(self, myId):
        self.myId = myId

    def getMyPlayer(self, state : 'State') -> 'Player':
        return state.players[ self.myId ]

    def getDrawAction(self, state : 'State') -> ActionOrResource:
        raise NotImplementedError

    def onBegin(self, state : 'State'):
        pass

    def getMovement(self, state : 'State') -> MoveDecision:
        raise NotImplementedError

    def getAbility(self, state : 'State') -> AbilityDecision:
        return AbilityDecision()

    def getAction(self, state : 'State') -> ActionDecision:
        raise NotImplementedError

    def drawAndDiscardStep(self):
        x : ActionOrResource = self.getDrawAction()
        if x == ActionOrResource.RESOURCES:
            self.resources += self.game.resDeck.draw()
            if len(self.resources) > MAX_RESOURCES:
                self.chooseAndDiscardResource()
        else:
            self.resources += self.actDeck.draw()
            if len(self.actions) > MAX_ACTIONS:
                self.chooseAndDiscardAction()

class HumanAgent(Agent):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def chooseCharacter(self, state : 'State'):
        for i, char in enumerate(state.aliveUnits[ self.myId ]):
            if char is not None:
                print(f'[{i}]: {char.name}')
        iSel = int(input('Enter which character to select: '))
        return state.aliveUnits[ self.myId ][iSel]

    def getDrawAction(self, state : 'State') -> ActionOrResource:
        iSel = input('Choose [1] draw action or [2] draw resource: ')
        return ActionOrResource.ACTION if iSel == '1' else ActionOrResource.RESOURCES

    def getMovement(self, state : 'State') -> MoveDecision:
        ARROWS = '^<>v'
        assert ARROWS[Direction.LEFT.value] == '<' and ARROWS[Direction.RIGHT.value] == '>'
        while True:
            charSel = self.chooseCharacter(state)
            possibleMovs = state.allMovementsForCharacter(charSel)
            if len(possibleMovs) == 0:
                print('No moves available for this unit. Choose another one.')
            else:
                for i, mov in enumerate(possibleMovs):
                    movString = ', '.join([ ARROWS[m.value] for m in mov[0] ]);
                    spaces = ' ' * 3 * ( charSel.mov - len(mov[0]) )
                    print(f'[{i}]: { movString + spaces } -> { mov[1] }')
                break
        iSel = int(input('Enter which move to select: '))
        movSel = possibleMovs[iSel]
        return MoveDecision(charSel.position, movSel[0], movSel[1])

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
                ret.subjectPos = state.aliveUnits[ self.myId ][ array[iSel - 1][0] ].position
                ret.objectPos = array[iSel - 1][1].position
            return ret

def manhattanDistance( pos1, pos2 ):
    return abs( pos1[0] - pos2[0] ) + abs( pos1[1] - pos2[1] )

def hashPosition( pos ):
    return chr( pos[0] * 10 + pos[1] + ord('a') )

def hashBoard( myAliveUnits ):
    return ''.join([ (hashPosition(unit.position) if (unit is not None) else ' ') for unit in myAliveUnits ])

ARROWS = '^<>v'
def printDecision( moveDecision ):
    return '' + str(moveDecision.frm) + (', '.join([ ARROWS[m.value] for m in moveDecision.moves ])) + str(moveDecision.to)

print('Board:')
print('-' * (FULL_BOARD_WIDTH*2 + 1))
for row in range(2):
    rowstr = '|'
    for col in range(FULL_BOARD_WIDTH):
        rowstr += hashPosition((row, col)) + '|'
    print(rowstr)
    print('-' * (FULL_BOARD_WIDTH*2 + 1))

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
            ally = oldState.getBoardField( step.kwargs['subject'] )
            nbTurnsBeforeAttack = (minDistance - enemy.rng)
            if nbTurnsBeforeAttack <= 0:
                nbTurnsBeforeAttack = 1
            danger = lostHP * 2 + math.ceil( enemy.mov / nbTurnsBeforeAttack ) # - lostLife
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
        #print('----ON BEGIN----')
        assert state.iActive == self.myId
        (state, decisions, heuristic) = self.planAhead( state, 0 )
        #print('Found', state, decisions, heuristic)
        #decision = [ dec for dec in decisions if isinstance(dec, ActionDecision) ][0]
        #print('ActionDecision:', decision.card)
        self.plans = decisions
    def planAhead(self, state : 'State', maxDepth : int):
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

            #print('nbPaths is', nbPaths)
            #if nbPaths > 10000:
                #raise Exception()
            #print("unloading step of type ", state.timestep, '@', active[3])
            #print('maxHeur is', maxHeur)
#            print('with cards:', [ len(i.actionDeck.cards) for i in state.players ])
#            print( 'History', active[1] )

            if state.timestep == Timestep.BEGIN:
                decision = ActionOrResource.ACTION
                (newState, step) = state.stepDraw( decision )
                stack.append( (newState, active[1] + [ decision ], active[2] + self.evaluateStep( state.iActive, state, step ), active[3] ) )
            elif state.timestep == Timestep.DISCARDED:
                print('Begin allMov phase')
                allPossibleMoves = {}
                substack = [ (state, 2, []) ]
                nbChildrenFiltered = 0
                while substack: #not empty
                    ( substate, nbMovRemaining, decisions ) = substack.pop()
                    stateid = hashBoard(substate.aliveUnits[substate.iActive])
                    #print('with', [ printDecision(decision) if decision else None for decision in decisions ], ', stateid would be ', stateid)
                    if stateid not in allPossibleMoves:
                        newSubState = substate.copy()
                        decisionsCompleteList = decisions
                        if nbMovRemaining != 0:
                            decisionsCompleteList = decisions + [None]
                            (newSubState, step) = newSubState.stepMov(None)
                            assert step.type == 'pass'
                        allPossibleMoves[ stateid ] = (newSubState, decisionsCompleteList)
                    else:
                        nbChildrenFiltered += 1
                    if nbMovRemaining != 0:
                        for charSel in substate.aliveUnits[ substate.iActive ]:
                            if charSel is not None:
                                possibleMovs = substate.allMovementsForCharacter(charSel)
                                for movSel in possibleMovs:
                                    decision = MoveDecision( charSel.position, movSel[0], movSel[1] )
                                    (newState, step) = substate.stepMov( decision )
                                    substack.append( (newState, nbMovRemaining - 1, decisions + [ decision ] ) )
                                    #print('Packing', nbMovRemaining - 1, decisions + [ decision ])
                for bundle in allPossibleMoves.values():
                    (newState, decisions) = bundle
                    heuristic_diff = 0 # TODO add step or state evaluation
                    stack.append( (newState, active[1] + decisions, active[2] + heuristic_diff, active[3] ) )
                print('End allMov phase! Same-state filtering kept', len(allPossibleMoves), 'out of', nbChildrenFiltered + len(allPossibleMoves) )

            elif state.timestep == Timestep.MOVEDlast:
                decision = AbilityDecision()
                (newState, step) = state.stepAbil( decision )
                stack.append( (newState, active[1] + [ decision ], active[2] + self.evaluateStep( state.iActive, state, step ), active[3] ) )
            elif state.timestep == Timestep.ABILITYCHOSEN:
                (newState, step) = state.stepAct(None)
                stack.append( (newState, active[1] + [ None ], active[2] + self.evaluateStep( state.iActive, state, step ), active[3]) ) # the "pass" option
                #print('Appending to stack "pass"', len(stack))
                ret = ActionDecision(None, None, None)
                #nbChildren = 0
                for card in state.players[ state.iActive ].actions:
                    ret.card = card
                    if card == ActionCard.DEFENSE:
                        for subject in state.aliveUnits[ state.iActive ]:
                            if subject is not None:
                                ret.subjectPos = subject.position
                                ret.object = None
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, active[1] + [ ret.copy() ], active[2] + self.evaluateStep( state.iActive, state, step ), active[3]) )
                                #print('Appending defense to stack', len(stack))
                                #nbChildren += 1
                    else:
                        allPossibleAttacks = state.allAttacks()
                        for aggressor in allPossibleAttacks:
                            for victim in allPossibleAttacks[aggressor]:
                                ret.subjectPos = state.aliveUnits[ state.iActive ][ aggressor ].position
                                ret.objectPos = victim.position
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, active[1] + [ ret.copy() ], active[2] + self.evaluateStep( state.iActive, state, step ), active[3]) )
                                #print('Appending attack to stack', len(stack))
                                #nbChildren += 1
            elif state.timestep == Timestep.ACTED:
                (newState, step) = state.beginTurn()
                if active[3] != maxDepth:
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
                        (newState, decisions, heuristic) = self.planAhead( newState, 0 )
                        print('RETURN TO MAXING')
                        #print(decisions, heuristic)
                        active = ( newState, active[1], active[2] - heuristic, active[3] + 2 )
                        if active[3] + 2 <= maxDepth:
                            stack.append( active )
                            continue
                        #else fallthrough
                nbPaths += 1
                #print('One path found! Heuristic:', active[2], ', max', maxHeur)
                #print('Stacksize', len(stack))
                if maxHeur is None or active[2] > maxHeur:
                    #print('Best path found! Heuristic:', active[2], ', max', maxHeur)
                    bestMoves = active[1]
                    bestState = newState
                    maxHeur = active[2]
        print(nbPaths, 'possible futures found')
        return ( bestState, bestMoves, maxHeur )
