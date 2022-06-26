from player import Player
from team import Team
from character import Character
from constants import Timestep, FULL_BOARD_WIDTH, HALF_BOARD_WIDTH, Faction, Direction, getNeighbour
from card import Deck, ActionCard
from serialize import Serializable
from agent import ActionOrResource
import copy

class Step(Serializable):
    def __init__(self, typ, **kwargs):
        self.type = typ
        self.kwargs = kwargs
    def __repr__(self):
        return f'Type: {self.type}; { self.kwargs }'
    def serialize(self):
        return { 'action' : self.type, **self.kwargs }

class State:
    def __init__(self, board, units, players, resDeck, timestep, turnId):
        self.board = board
        self.players = players
        self.resDeck = resDeck
        self.aliveUnits = units
        self.nbAliveUnits = [ len(army) for army in units ]
        self.iActive = 0
        self.timestep = timestep
        self.turnId = turnId

    @staticmethod
    def createStart(teams) -> 'State':
        board = [ [None] * FULL_BOARD_WIDTH for _ in range(2) ]
        alive = [ [], [] ]
        for (i, team) in enumerate(teams):
            for (j, row) in enumerate(team.characters):
                for (k, char) in enumerate(row):
                    char.teampos = len(alive[i])
                    alive[i].append(char)
                    board[ j ][ k + 1 + HALF_BOARD_WIDTH*i ] = char
                    char.position = ( j, k + 1 + HALF_BOARD_WIDTH*i )
                    char.team = i

        players = [ Player() for i in range(2) ]

        for player in players:
            for i in range(2):
                player.drawAction()

        startingResources = sum([[x] * 4 for x in Faction.allFactions()], []) + [ Faction.ETHER ]
        resDeck = Deck.create(startingResources)

        return State(board, alive, players, resDeck, Timestep.BEGIN, 0)

    def copy(self):
        #ret = State(self.board, self.aliveUnits, self.players, self.resDeck, self.timestep)
        #ret.iActive = self.iActive
        #return ret
        return copy.copy(self)

    def getBoardField(self, coords):
        return self.board[ coords[0] ][ coords[1] ]
    def setBoardField(self, coords, value):
        #self.checkConsistency()
        #assert self.getBoardField(coords) is None
        self.board[ coords[0] ][ coords[1] ] = value
        if(value):
            value.position = coords
            self.aliveUnits[value.team][value.teampos] = value
        #self.checkConsistency()

    def isFinished(self):
        return self.nbAliveUnits[0] == 0 or self.nbAliveUnits[1] == 0

    def stepDraw(self, decision : ActionOrResource):
        assert self.timestep == Timestep.BEGIN
        self.checkConsistency()
        newState = self.copy()
        newState.timestep = Timestep.DISCARDED
        newState.players = newState.players.copy()
        newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
        newState.checkConsistency()
        if decision == ActionOrResource.ACTION:
            newState.players[self.iActive].actions = newState.players[self.iActive].actions.copy()
            newState.players[self.iActive].actionDeck = newState.players[self.iActive].actionDeck.copy()
            cardDrawn = newState.players[self.iActive].drawAction()
            return ( newState, Step( typ='draw', clss='action', value=cardDrawn, newDeckSize=newState.players[self.iActive].actionDeck.sizeconfig() ) )
        else:
            newState.resDeck = newState.resDeck.copy()
            newState.players[self.iActive].resources = newState.players[self.iActive].resources.copy()
            cardDrawn = newState.players[self.iActive].drawResource(newState.resDeck)
            return ( newState, Step( typ='draw', clss='resource', value=cardDrawn, newDeckSize=newState.resDeck.sizeconfig() ) )

    def checkConsistency(self):
        for team in self.aliveUnits:
            for char in team:
                if char is not None and self.getBoardField( char.position ) is not char:
                    print('!Error:', self.getBoardField( char.position ), '@', char.position, 'is not', char)
                    print( [ [ (char.cid, char.position) if char else None for char in row ] for row in self.aliveUnits ] )
                    print([ [ () if char is None else (char.cid, char.position) for char in row ] for row in self.board ])
                    raise Exception()

    def stepMov(self, decision : 'MoveDecision'):
        self.checkConsistency()
        newState = self.copy()
        newState.checkConsistency()
        if self.timestep == Timestep.DISCARDED:
            newState.timestep = Timestep.MOVEDfirst
        elif self.timestep == Timestep.MOVEDfirst:
            newState.timestep = Timestep.MOVEDlast
        else:
            raise AssertionError()
        if decision is None:
            newState.timestep = Timestep.MOVEDlast
            return ( newState, Step( typ='pass', message='Did not move' ) )
        else:
            #print("board is", [ [ () if char is None else (char.cid, char.position) for char in row ] for row in self.board ])
            # Invalidating
            newState.board = [ row.copy() for row in self.board ]
            newState.aliveUnits = [ row.copy() for row in self.aliveUnits ]

            # mover is always an old-reference, until the moment where it is inserted
            mover = newState.getBoardField(decision.frm)
            mover.turnMoved = newState.turnId
            newState.setBoardField(decision.frm, None)
            landingSpot = decision.to
            index = len(decision.moves)
            while True:
                moved = newState.getBoardField(landingSpot)
                newState.setBoardField(landingSpot, mover.copy())
                if moved is None:
                    break
                index -= 1
                mover = moved
                landingSpot = getNeighbour( landingSpot, decision.moves[index].inverse() )

            #print(decision.frm, 'to', decision.to)
            #print("newState.board is", [ [ () if char is None else (char.cid, char.position) for char in row ] for row in newState.board ])

            newState.checkConsistency()

            return ( newState, Step(
                            typ='move', frm=decision.frm, to=decision.to, target=(self.getBoardField(decision.frm).cid), 
                            moves=decision.moves, firstCOF=index
                    ) )

    def stepAbil(self, decision : 'AbilityDecision'):
        assert self.timestep == Timestep.MOVEDlast
        newState = self.copy()
        if decision.type == 'pass':
            newState.timestep = Timestep.ABILITYCHOSEN
            return (newState, Step(typ='pass', message='Abilities not implemented yet'))
        else:
            raise NotImplementedError()

    def stepAct(self, decision : 'ActionDecision'):
        assert self.timestep == Timestep.ABILITYCHOSEN
        newState = self.copy()
        newState.timestep = Timestep.ACTED
        if decision is None:
            return (newState, Step(typ='pass', message='No action chosen'))

        # Invalidating
        newState.board = [ row.copy() for row in self.board ]
        newState.aliveUnits = [ row.copy() for row in self.aliveUnits ]
        newState.players = newState.players.copy()
        newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
        newState.players[self.iActive].actionDeck = newState.players[self.iActive].actionDeck.copy()
        newState.players[self.iActive].actions = newState.players[self.iActive].actions.copy()

        newState.players[self.iActive].discard(decision.card)

        hero = newState.getBoardField(decision.subjectPos).copy()
        newState.setBoardField( hero.position, hero )
        newState.checkConsistency()
        if decision.card == ActionCard.DEFENSE:
            newShieldHP = hero.buff()
            return (newState, Step(typ='def', cardLost='defense', subject=decision.subjectPos, temporary=newShieldHP, permanent=50))
        else:
            hero.turnAttacked = newState.turnId
            setLife = 0
            victim = newState.getBoardField(decision.objectPos).copy()
            if decision.card == ActionCard.SOFTATK:
                lostLife = victim.takeDmg(False, hero.getAtk(False, newState.turnId))
                ret = Step(typ='atk', cardLost='softAtk', subject=decision.subjectPos, object=decision.objectPos, setLife=victim.HP, lostLife=lostLife)
            elif decision.card == ActionCard.HARDATK:
                lostLife = victim.takeDmg(True, hero.getAtk(True, newState.turnId))
                ret = Step(typ='atk', cardLost='hardAtk', subject=decision.subjectPos, object=decision.objectPos, setLife=victim.HP, lostLife=lostLife)
            else:
                raise AssertionError('decision.card is neither hard, soft, nor defense')
            if victim.HP <= 0:
                newState.nbAliveUnits = self.nbAliveUnits.copy()
                newState.nbAliveUnits[ 1 - self.iActive ] -= 1
                ret.kwargs['delete'] = True
                newState.setBoardField( victim.position, None)
                newState.aliveUnits[ 1 - self.iActive ][ victim.teampos ] = None
            else:
                newState.setBoardField( victim.position, victim )
            newState.checkConsistency()
            return (newState, ret)

    def beginTurn(self):
        self.checkConsistency()
        assert self.timestep == Timestep.ACTED
        ret = self.copy()
        ret.iActive = 1 - ret.iActive
        ret.timestep = Timestep.BEGIN
        dirty = False
        for (i, unit) in enumerate(ret.aliveUnits[ret.iActive]):
            if unit is not None:
                clone = unit.beginTurn()
                if clone is not unit:
                    if not dirty:
                        ret.board = [ row.copy() for row in ret.board ]
                        ret.aliveUnits = [ row.copy() for row in ret.aliveUnits ]
                        dirty = True
                    ret.setBoardField(clone.position, clone)
                    ret.aliveUnits[ret.iActive][i] = clone
        ret.checkConsistency()
        return (ret, Step(typ='beginturn'))

    def allMovementsForCharacter(self, character : Character):
        if self.timestep == Timestep.MOVEDfirst and character.turnMoved == self.turnId:
            return []
        ret = []
        stack = [ (character.position, False, []) ]
        boardOfBools = [ [ False for i in range(FULL_BOARD_WIDTH) ] for j in range(2) ] # whether that field has already been passed in the current iteration or not
        while stack: # not empty
            active = stack.pop()
            pos = active[0]
            if active[1] is False: # first time we pass this
                if active[2]: # do not select the empty list, aka "staying in place"
                    ret.append( (active[2], active[0]) )
                if len(active[2]) < character.mov:
                    stack.append( (active[0], True, active[2]) )
                    boardOfBools[pos[0]][pos[1]] = True
                    neighbours = []
                    if pos[1] != 0:
                        neighbours.append( (Direction.LEFT, getNeighbour(pos, Direction.LEFT)) )
                    if pos[1] != FULL_BOARD_WIDTH-1:
                        neighbours.append( (Direction.RIGHT, getNeighbour(pos, Direction.RIGHT)) )
                    neighbours.append( ((Direction.DOWN if pos[0] == 0 else Direction.UP), (1 - pos[0], pos[1])) )
                    for neighbour in neighbours:
                        if boardOfBools[neighbour[1][0]][neighbour[1][1]]:
                            continue
                        resident = self.getBoardField(neighbour[1])
                        if resident is not None and resident.team != character.team:
                            continue
                        stack.append( (neighbour[1], False, active[2] + [ neighbour[0] ]) )
            else: # second pass
                boardOfBools[pos[0]][pos[1]] = False
        return ret

    def allAttacks(self):
        assert self.timestep == Timestep.ABILITYCHOSEN
        ret = {}
#        print(f' iActive : { self.iActive }, units : { [unit.name for unit in self.aliveUnits[ self.iActive ] ] }')
        for iChar, character in enumerate( self.aliveUnits[ self.iActive ] ):
            if character is not None:
                created : bool = False
                for row in range(2):
                    deltaRow = abs( character.position[0] - row )
                    for column in range( max(character.position[1] - character.rng + deltaRow, 0), min(character.position[1] + character.rng - deltaRow + 1, FULL_BOARD_WIDTH) ):
                        if self.board[row][column] is not None and self.board[row][column].team != self.iActive:
                            if not created:
                                created = True
                                ret[ iChar ] = []
                            ret[ iChar ].append( self.board[row][column] )
        return ret

    def advance(self, agent) -> 'State':
        if self.timestep == Timestep.ACTED:
            return self.beginTurn()
        elif self.timestep == Timestep.BEGIN:
            decision = agent.getDrawAction(self)
            return self.stepDraw(decision)
        elif self.timestep == Timestep.DISCARDED or self.timestep == Timestep.MOVEDfirst:
            decision = agent.getMovement(self)
            return self.stepMov(decision)
        elif self.timestep == Timestep.MOVEDlast:
            decision = agent.getAbility(self)
            return self.stepAbil(decision)
        elif self.timestep == Timestep.ABILITYCHOSEN:
            decision = agent.getAction(self)
            return self.stepAct(decision)

    def serialize(self):
        return {
            "resourceDeck" : self.resDeck.sizeconfig(),
            "players" : [ player.serialize() for player in self.players ],
            "board": [ [ (char.serialize() if char else None ) for char in row ] for row in self.board ]
        }
