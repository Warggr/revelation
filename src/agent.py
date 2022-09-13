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
        print('Read', iSel)
        if iSel == 0:
            return None
        else:
            ret.card = cards[iSel - 1]
            print(ret.card)
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
                    for enemyIndex in allPossibleAttacks[key]:
                        enemy = state.aliveUnits[ 1 - self.myId ][enemyIndex]
                        startCharacter = '└' if enemyIndex == allPossibleAttacks[key][-1] else '├';
                        print(f'[{ i }]{ startCharacter }─{ enemy.name }')
                        array.append( [ key, enemy ] )
                        i += 1
                iSel = int(input('Enter which attack to select: ')) - 1
                ret.subjectPos = state.aliveUnits[ self.myId ][ array[iSel][0] ].position
                ret.objectPos = array[iSel][1].position
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

"""print('Board:')
print('-' * (FULL_BOARD_WIDTH*2 + 1))
for row in range(2):
    rowstr = '|'
    for col in range(FULL_BOARD_WIDTH):
        rowstr += hashPosition((row, col)) + '|'
    print(rowstr)
    print('-' * (FULL_BOARD_WIDTH*2 + 1))"""

class ProgressLogger:
    def enterTurn(self):
        pass
    def exitTurn(self):
        pass
    def enter(self, timestep, nbChildren):
        pass
    def exit(self, timestep):
        pass
    def message(self, *args, **kwargs):
        print(*args, **kwargs)

class SimpleIndentLogger(ProgressLogger):
    def __init__(self):
        self.indent = ''
    def enterTurn(self):
        self.indent += '    '
    def exitTurn(self):
        self.indent = self.indent[:-4]
    def message(self, *args, **kwargs):
        print(self.indent, *args, **kwargs)

class ProgressBar(ProgressLogger):
    def __init__(self):
        self.progressStack = []
        self.progress = []
        self.buffer = ''
    def message(self, *args, **kwargs):
        pass # suppress output
    def enter(self, timestep, nbChildren):
        if timestep == Timestep.DISCARDED:
            self.buffer += (f'[{nbChildren:4}\\   1]->')
            #sys.stdout.flush()
            self.progress.append(0)
        elif timestep == Timestep.MOVEDlast:
            self.progress[-1] += 1
            self.buffer += ('\b' * 7 + f'{self.progress[-1]:4}]->')
            #sys.stdout.flush()
        elif timestep == Timestep.ABILITYCHOSEN:
            self.buffer += (f'<{nbChildren:4}\\   1>->')
            #sys.stdout.flush()
            self.progress.append(0)
        elif timestep == Timestep.ACTED:
            self.progress[-1] += 1
            self.buffer += ('\b' * 7 + f'{self.progress[-1]:4}>->')
            #sys.stdout.flush()
    def exit(self, timestep):
        if timestep == Timestep.DISCARDED or timestep == Timestep.ABILITYCHOSEN:
            self.buffer += ('\b' * 13) # deleting my progress-frame
            self.progress.pop()
    def enterTurn(self):
        self.progressStack.append(self.progress)
        self.progress = []
    def exitTurn(self):
        self.buffer += '\b' * 13 * len(self.progress) # deleting progress-frames for all states in the turn which were not exited yet
        self.progress = self.progressStack.pop()
        sys.stdout.write(self.buffer)
        sys.stdout.flush()
        self.buffer = ''

class AIAgent(Agent):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.plans = []
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
    @staticmethod
    def pushChildStates(stackframe, putback, stateCache):
        """
        Creates all children states of state (the first element in the @param stackframe) and adds them into the data structure @param putback.
        @returns the number of children inserted this way.
        """
        (state, ac_decisionhistory, ac_heuristic, ac_depth, pass2) = stackframe
        if state.timestep == Timestep.BEGIN:
            decision = ActionOrResource.ACTION
            (newState, step) = state.stepDraw( decision )
            putback.append( (newState, ac_decisionhistory + [ decision ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False ) )
            return 1
        elif state.timestep == Timestep.DISCARDED:
            #print('Begin allMov phase')
            allPossibleMoves = {}
            substack = [ (state, 2, []) ]
            #nbChildrenFiltered = 0
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
                #else:
                    #nbChildrenFiltered += 1
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
                putback.append( (newState, ac_decisionhistory + decisions, ac_heuristic + heuristic_diff, ac_depth, False ) )
            #print('End allMov phase! Same-state filtering kept', len(allPossibleMoves), 'out of', nbChildrenFiltered + len(allPossibleMoves) )
            return len(allPossibleMoves)

        elif state.timestep == Timestep.MOVEDlast:
            decision = AbilityDecision()
            (newState, step) = state.stepAbil( decision )
            putback.append( (newState, ac_decisionhistory + [ decision ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) )
            return 1

        elif state.timestep == Timestep.ABILITYCHOSEN:
            (newState, step) = state.stepAct(None)
            putback.append( (newState, ac_decisionhistory + [ None ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) ) # the "pass" option
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
                            putback.append( (newState, ac_decisionhistory + [ ret.copy() ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) )
                            #print('Appending defense to stack', len(stack))
                            nbChildren += 1
                else:
                    allPossibleAttacks = state.allAttacks()
                    for aggressor in allPossibleAttacks:
                        for victim in allPossibleAttacks[aggressor]:
                            ret.subjectPos = state.aliveUnits[ state.iActive ][ aggressor ].position
                            ret.objectPos = state.aliveUnits[ 1 - state.iActive ][ victim ].position
                            (newState, step) = state.stepAct( ret )
                            putback.append( (newState, ac_decisionhistory + [ ret.copy() ], ac_heuristic + SearchAgent.evaluateStep( state.iActive, state, step ), ac_depth, False) )
                            #print('Appending attack to stack', len(stack))
                            nbChildren += 1
            return nbChildren

        elif timestep == Timestep.ACTED:
            raise Exception('Timestep.ACTED (aka end of turns) are supposed to be handled differently')

# interface / fully abstract class
class DepthPolicy:
    def enterOpponentsTurn(self):
        pass
    def enterOwnTurn(self):
        return None # depth policies by default do not allow simulating my own turn after the opponent's simulation
    def asTuple(self):
        pass
    def informNbChildren(self, nbChildren, timestepLevel):
        pass

class FixedDepthPolicy(DepthPolicy):
    """
    Search up to a fixed depth described by a set of parenthesis.
        - [] indicates that only my own turn will be considered
        - [ X ] my own turn will be considered, then the opponent will choose their best state with maxDepth=X
        - [ X, Y ] I will simulate my own turn, then the opponent will simulate their turn with maxDepth=X and return the result, then I will simulate the consequences of their turn with maxDepth=Y.

        For example, for a full-2-turns-ahead simulation, use maxDepth = [ [ [ [] ] ] ]
    """
    def __init__(self, parentheses):
        self.parentheses = parentheses
    def enterOpponentsTurn(self):
        if len(self.parentheses) < 1:
            return None
        else:
            return FixedDepthPolicy(self.parentheses[0])
    def enterOwnTurn(self):
        if len(self.parentheses) < 2:
            return 2
        else:
            return FixedDepthPolicy(self.parentheses[1])
    @staticmethod
    def _asTuple(parentheses):
        myTurns, opponentsTurns = (1, 0)
        if len(parentheses) >= 1:
            (opp, me) = FixedDepthPolicy._asTuple(parentheses[0])
            myTurns += me
            opponentsTurns += opp
            if len(parentheses) == 2:
                (me, opp) = FixedDepthPolicy._asTuple(parentheses[1])
                myTurns += me
                opponentsTurns += opp
        return ( myTurns, opponentsTurns )
    def asTuple(self):
        return FixedDepthPolicy._asTuple(self.parentheses)

class AdaptiveDepthPolicy(DepthPolicy):
    usedLevelsMap = [ 0, -1, 1, -1, 2, 3 ]
    nbUsedLevels = 4

    def __init__(self, maxNodes = 1000000):
        self.maxNodes = maxNodes
        self.nodes = 1
        self.sumChildren = [ 0 ] * AdaptiveDepthPolicy.nbUsedLevels
        self.nbChildren = [ 0 ] * AdaptiveDepthPolicy.nbUsedLevels
        self.currentChildrenCount = [ 0 ] * AdaptiveDepthPolicy.nbUsedLevels
        self.currentLevel = -1
    def informNbChildren(self, nbChildren, timestepLevel):
        # print(timestepLevel.value, '->', AdaptiveDepthPolicy.usedLevelsMap[ timestepLevel.value ], '->', nbChildren)
        timestepLevel = AdaptiveDepthPolicy.usedLevelsMap[ timestepLevel.value ]
        if timestepLevel == self.currentLevel + 1:
            pass
        elif timestepLevel == self.currentLevel:
            self.sumChildren[ timestepLevel ] += self.currentChildrenCount[ timestepLevel ]
            self.nbChildren[ timestepLevel ] += 1
        elif timestepLevel == self.currentLevel - 1:
            self.sumChildren[ timestepLevel ] += self.sumChildren[ self.currentLevel ] + self.currentChildrenCount[ self.currentLevel ] + 1
            self.nbChildren[ timestepLevel ] += 1
            self.sumChildren[ self.currentLevel ] = 0
            self.nbChildren[ self.currentLevel ] = 0
        else:
            raise AssertionError()
        self.currentLevel = timestepLevel
        self.currentChildrenCount[ timestepLevel ] = nbChildren
        # print('currentChildrenCount:', self.currentChildrenCount)
        # print('sumChildren:', self.sumChildren)
        # print('nbChildren:', self.nbChildren)
        # print('Current level:', self.currentLevel)
        # input()
    def estimateNbBranches(self):
        # print('currentChildrenCount:', self.currentChildrenCount)
        # print('sumChildren:', self.sumChildren)
        # print('nbChildren:', self.nbChildren)
        # print('Current level:', self.currentLevel)
        nbBranches = self.currentChildrenCount[ self.currentLevel ]
        for i in range(self.currentLevel, 0, -1):
            # print(f'( {nbBranches} + {self.sumChildren[ i ]} ) * ( {self.currentChildrenCount[ i - 1 ]} / ({self.nbChildren[i]} + 1) )', end='')
            nbBranches = ( nbBranches + self.sumChildren[ i ] ) * ( self.currentChildrenCount[ i - 1 ] / (self.nbChildren[i] + 1) ) + 1
            # print(f' =: {nbBranches}')
        return nbBranches
    def enterOpponentsTurn(self):
        #print('To enter or not to enter, that is the question...')
        nodes = self.estimateNbBranches()
        # print('MaxNodes', self.maxNodes, 'at this depth there were', nodes)
        if self.maxNodes <= 1.5 * (nodes ** 2):
            return None
        else:
            # print('Entering!')
            # print('currentChildrenCount:', self.currentChildrenCount)
            # print('sumChildren:', self.sumChildren)
            # print('nbChildren:', self.nbChildren)
            # print('Current level:', self.currentLevel)
            return AdaptiveDepthPolicy(self.maxNodes / nodes)
    def asTuple(self):
        log = 0
        i = self.maxNodes
        while i > 1:
            i = i // self.nodes - 1
            log += 1
        return ( log // 2 + log % 2, log // 2 )

class SearchAgent(AIAgent):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    @staticmethod
    def evaluateMaxForState(state, playerId, nbTurnsRemaining):
        myMaxAtk = 0
        for unit in state.aliveUnits[ playerId ]:
            if unit is not None:
                myMaxAtk = unit.maxAtk
                break
        nbMaxDamage = myMaxAtk * nbTurnsRemaining
        heuristic = 0
        for enemy in state.aliveUnits[ 1 - playerId ]:
            if unit is not None:
                if unit.HP < nbMaxDamage:
                    heuristic += unit.maxAtk * ( unit.HP + unit.maxHP )
                    nbMaxDamage -= unit.HP
                else:
                    heuristic += unit.maxAtk * nbMaxDamage
                    break
        return heuristic
    @staticmethod
    def evaluateStep(myId, oldState, step):
        if step.type == 'atk':
            obj = oldState.getBoardFieldDeref( step.kwargs['object'] )
            ret = step.kwargs[ 'lostLife' ] * obj.maxAtk
            if 'delete' in step.kwargs:
                ret += obj.maxAtk * obj.maxHP
            return ret
        elif step.type == 'def':
            return 50 * oldState.getBoardFieldDeref( step.kwargs['subject'] ).maxAtk
        elif step.type == 'move':
            return -1
        else:
            return 0
    def onBegin(self, state : 'State'):
        #print('----ON BEGIN----')
        assert state.iActive == self.myId
        (state, decisions, heuristic) = SearchAgent.planAhead( state, FixedDepthPolicy([]) )
        #print('Found', state, decisions, heuristic)
        #decision = [ dec for dec in decisions if isinstance(dec, ActionDecision) ][0]
        #print('ActionDecision:', decision.card)
        self.plans = decisions
    @staticmethod
    def planAhead(state : 'State', depthPolicy, maxHeurAllowed = None, progressLogger=ProgressBar(), stateCache = {}):
        """
        Constructs all descendants of @param state. up to a level defined by @param depthPolicy.
        Selects the one that is best for the active player (defined by state.iActive)
        Returns a tuple (state, decisions, heuristic) with the bext future state, the decisions that led to it, and the heuristic of that state.

        Specify @param maxHeurAllowed if you want the function to return directly if it reaches a certain heuristic.
        """
        progressLogger.enterTurn()
        #nbPaths = 0
        bestMoves = None
        bestState = None
        maxHeur = None
        worstOpponentsHeuristic = None
        stack = [ (state, [], 0, depthPolicy, False) ] #state, decision history, heuristic, depth, second-pass
        while stack: #not empty
            stackframe = stack.pop()
            (state, ac_decisionhistory, ac_heuristic, ac_depth, pass2) = stackframe
            if pass2:
                progressLogger.exit(state.timestep)
                continue
            else:
                stack.append((state, None, None, None, True))

            if state.timestep != Timestep.ACTED:
                nbChildren = AIAgent.pushChildStates(stackframe, stack, stateCache)
                progressLogger.enter(state.timestep, nbChildren)
                ac_depth.informNbChildren(nbChildren, state.timestep)
            else:
                (newState, step) = state.beginTurn()
                progressLogger.enter(state.timestep, 1)

                subDepth = ac_depth.enterOpponentsTurn()
                if subDepth is not None:
                    progressLogger.message(ac_depth, '->', ac_depth.asTuple())
                    (nbTurnsMe, nbTurnsOpponent) = ac_depth.asTuple()
                    # we're using state.iActive and not newState.iActive because in newState, iActive has already switched to the opponent
                    max_bound = ac_heuristic + SearchAgent.evaluateMaxForState(newState, 1 - state.iActive, nbTurnsOpponent)
                    min_bound = ac_heuristic - SearchAgent.evaluateMaxForState(newState, state.iActive, nbTurnsMe)
                    progressLogger.message(min_bound, ac_heuristic, max_bound)
                    if maxHeurAllowed is not None and min_bound >= maxHeurAllowed:
                        progressLogger.exitTurn()
                        return (None, None, min_bound)
                    if maxHeur is not None and max_bound <= maxHeur:
                        continue

                    progressLogger.message('MINNING with cut-off at', worstOpponentsHeuristic)
                    #print(len(stack))
                    (newState, _decisions, opponentsHeuristic) = SearchAgent.planAhead( newState, subDepth, worstOpponentsHeuristic, progressLogger ) # we don't care about the decisions
                    if newState is None: # this happens when the search was better than bestOpponentsHeuristic and was cut off
                        progressLogger.message('Search cut off')
                        continue
                    if worstOpponentsHeuristic is None or opponentsHeuristic < worstOpponentsHeuristic:
                        worstOpponentsHeuristic = opponentsHeuristic
                    ac_heuristic -= opponentsHeuristic
                    progressLogger.message('Search returned', opponentsHeuristic)
                    subsubDepth = ac_depth.enterOwnTurn()
                    if subsubDepth is not None:
                        active = ( newState, ac_decisionhistory, ac_heuristic, subsubDepth, progressLogger )
                        stack.append( active )
                        continue
                    #else fallthrough
                #nbPaths += 1
                #print('One path found! Heuristic:', active[2], ', max', maxHeur)
                #print('Stacksize', len(stack))

                if maxHeur is None or ac_heuristic > maxHeur:
                    #print('Best path found! Heuristic:', active[2], ', max', maxHeur)
                    bestMoves = ac_decisionhistory
                    bestState = newState
                    maxHeur = ac_heuristic
        #print(nbPaths, 'possible futures found')
        progressLogger.exitTurn()
        return ( bestState, bestMoves, maxHeur )
