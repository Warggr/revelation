from typing import Union
from enum import Enum, unique
from constants import MAX_RESOURCES, MAX_ACTIONS, Timestep, Direction, FULL_BOARD_WIDTH
from card import ActionCard
from character import Character
import sys

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
    @staticmethod
    def evaluateStep(myId, oldState, step):
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
        (state, decisions, heuristic) = self.planAhead( state,  [[[[]]]] )
        #print('Found', state, decisions, heuristic)
        #decision = [ dec for dec in decisions if isinstance(dec, ActionDecision) ][0]
        #print('ActionDecision:', decision.card)
        self.plans = decisions
    @staticmethod
    def planAhead(state : 'State', maxDepth, indent = ''):
        """
        Constructs all descendants of @param state. up to a level defined by @param maxDepth.
        Selects the one that is best for the active player (defined by state.iActive)
        Returns a tuple (state, decisions, heuristic) with the bext future state, the decisions that led to it, and the heuristic of that state.

        For param maxDepth:
        - [] indicates that only my own turn will be considered
        - [ X ] my own turn will be considered, then the opponent will choose their best state with maxDepth=X
        - [ X, Y ] I will simulate my own turn, then the opponent will simulate their turn with maxDepth=X and return the result, then I will simulate the consequences of their turn with maxDepth=Y.

        For example, for a full-2-turns-ahead simulation, use maxDepth = [ [ [ [] ] ] ]
        """
        #print(indent, 'Starting planAhead with depth', maxDepth)
        nbPaths = 0
        maxHeur = None
        maxHeurTemp = -20
        minTolerated = -20
        bestMoves = None
        bestState = None
        progress = []
        stack = [ (state, [], 0, maxDepth, False) ] #state, decision history, heuristic, depth, second-pass
        while stack: #not empty
            (state, ac_decisionhistory, ac_heuristic, ac_depth, pass2) = stack.pop()
            #print()
            #print(indent, state.timestep, progress, pass2)
            if pass2:
                if state.timestep == Timestep.DISCARDED or state.timestep == Timestep.ABILITYCHOSEN:
                    sys.stdout.write('\b' * 13) # deleting my progress-frame
                    progress.pop()
                    sys.stdout.flush()
                continue
            else:
                stack.append((state, None, None, None, True))

            if state.timestep == Timestep.BEGIN:
                decision = ActionOrResource.ACTION
                (newState, step) = state.stepDraw( decision )
                stack.append( (newState, ac_decisionhistory + [ decision ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False ) )
            elif state.timestep == Timestep.DISCARDED:
                #print('Begin allMov phase')
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
                    stack.append( (newState, ac_decisionhistory + decisions, ac_heuristic + heuristic_diff, ac_depth, False ) )
                #print(indent, 'End allMov phase! Same-state filtering kept', len(allPossibleMoves), 'out of', nbChildrenFiltered + len(allPossibleMoves) )
                sys.stdout.write(f'[{len(allPossibleMoves):4}\\   1]->')
                sys.stdout.flush()
                progress.append(0)

            elif state.timestep == Timestep.MOVEDlast:
                decision = AbilityDecision()
                (newState, step) = state.stepAbil( decision )
                stack.append( (newState, ac_decisionhistory + [ decision ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) )
                progress[-1] += 1
                sys.stdout.write('\b' * 7 + f'{progress[-1]:4}]->')
                sys.stdout.flush()

            elif state.timestep == Timestep.ABILITYCHOSEN:
                (newState, step) = state.stepAct(None)
                stack.append( (newState, ac_decisionhistory + [ None ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) ) # the "pass" option
                #print('Appending to stack "pass"', len(stack))
                ret = ActionDecision(None, None, None)
                nbChildren = 1 # already one child: the 'pass' decision
                for card in state.players[ state.iActive ].actions:
                    ret.card = card
                    if card == ActionCard.DEFENSE:
                        for subject in state.aliveUnits[ state.iActive ]:
                            if subject is not None:
                                ret.subjectPos = subject.position
                                ret.object = None
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, ac_decisionhistory + [ ret.copy() ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) )
                                #print('Appending defense to stack', len(stack))
                                nbChildren += 1
                    else:
                        allPossibleAttacks = state.allAttacks()
                        for aggressor in allPossibleAttacks:
                            for victim in allPossibleAttacks[aggressor]:
                                ret.subjectPos = state.aliveUnits[ state.iActive ][ aggressor ].position
                                ret.objectPos = victim.position
                                (newState, step) = state.stepAct( ret )
                                stack.append( (newState, ac_decisionhistory + [ ret.copy() ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) )
                                #print('Appending attack to stack', len(stack))
                                nbChildren += 1
                sys.stdout.write(f'<{nbChildren:4}\\   1>->')
                sys.stdout.flush()
                progress.append(0)
            elif state.timestep == Timestep.ACTED:
                progress[-1] += 1
                sys.stdout.write('\b' * 7 + f'{progress[-1]:4}>->')
                sys.stdout.flush()
                (newState, step) = state.beginTurn()
                if len(ac_depth) != 0:
                    #print(indent, 'MINNING')
                    #print(len(stack))
                    #print(heuristic, ', tolerate', minTolerated, '-', maxHeurTemp)
                    (newState, decisions, heuristic) = SearchAgent.planAhead( newState, ac_depth[0], indent+'    ' )
                    #print(indent, 'RETURN TO MAXING')
                    #print(decisions, heuristic)
                    if len(ac_depth) == 2:
                        active = ( newState, ac_decisionhistory, ac_heuristic - heuristic, ac_depth[1], indent+'    ' )
                        stack.append( active )
                        continue
                    #else fallthrough
                nbPaths += 1
                #print('One path found! Heuristic:', active[2], ', max', maxHeur)
                #print('Stacksize', len(stack))
                if maxHeur is None or ac_heuristic > maxHeur:
                    #print('Best path found! Heuristic:', active[2], ', max', maxHeur)
                    bestMoves = ac_decisionhistory
                    bestState = newState
                    maxHeur = ac_heuristic
        #print(indent, nbPaths, 'possible futures found')
        return ( bestState, bestMoves, maxHeur )
