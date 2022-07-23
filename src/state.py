from player import Player
from team import Team
from character import Character
from constants import Timestep, FULL_BOARD_WIDTH, HALF_BOARD_WIDTH, ARMY_SIZE, Faction, Direction, getNeighbour, DEAD_UNIT, EMPTY_FIELD, isEmpty, isDead
from card import Deck, ActionCard
from serialize import Serializable
from agent import ActionOrResource
import copy

class BoardTile:
    def __init__(self, team, index):
        self.team = team
        self.index = index
    def __eq__(self, other):
        if isDead(other):
            return False
        return other.team == self.team and other.index == self.index
    def __repr__(self):
        return f't{self.team}-i{self.index}'
    def serialize(self):
        return [ self.team, self.index ]

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
        self.nbAliveUnits = [ ARMY_SIZE ] * 2
        self.iActive = 0
        self.timestep = timestep
        self.turnId = turnId

    @staticmethod
    def createStart(teams) -> 'State':
        board = [ [EMPTY_FIELD] * FULL_BOARD_WIDTH for _ in range(2) ]
        alive = [ [], [] ]
        for (i, team) in enumerate(teams):
            for (j, row) in enumerate(team.characters):
                for (k, char) in enumerate(row):
                    char.teampos = len(alive[i])
                    alive[i].append(char)
                    char.position = ( j, k + 1 + HALF_BOARD_WIDTH*i )
                    char.team = i

        for iTeam, team in enumerate(alive):
            team.sort(reverse=True, key=lambda crea: crea.maxAtk) # sorting units by their maximum attack (useful for faster heuristic calculations)
            for iChar, char in enumerate(team):
                char.teampos = iChar
                board[char.position[0]][char.position[1]] = BoardTile(iTeam, iChar)

        players = [ Player() for i in range(2) ]

        for player in players:
            for i in range(2):
                player.drawAction()

        startingResources = sum([[x] * 4 for x in Faction.allFactions()], []) + [ Faction.ETHER ]
        resDeck = Deck.create(startingResources)

        return State(board, alive, players, resDeck, Timestep.BEGIN, 0)

    def copy(self):
        return copy.copy(self)

    def getBoardField(self, coords):
        return self.board[ coords[0] ][ coords[1] ]
    def getBoardFieldDeref(self, coords):
        ptr = self.getBoardField(coords)
        if isEmpty(ptr):
            return False
        return self.aliveUnits[ptr.team][ptr.index]
    def setBoardField(self, coords, value):
        self.board[ coords[0] ][ coords[1] ] = value
    def setBoardFieldDeref(self, coords, value):
        self.setBoardField(coords, value)
        if not isEmpty(value):
            self.aliveUnits[value.team][value.index].position = coords

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
        for iTeam, team in enumerate(self.aliveUnits):
            for index in range(ARMY_SIZE):
                if not isDead(team[index]) and self.getBoardField( team[index].position ) != BoardTile(iTeam, index):
                    print(f'!Error: {team[index].cid} @ {index},team{iTeam} has position {team[index].position}, which itself contains a reference to {self.getBoardField( team[index].position )}')
                    print( [ [ None if isDead(char) else (char.cid, char.position) for char in row ] for row in self.aliveUnits ] )
                    print([ [ () if isEmpty(char) else (char.team, char.index) for char in row ] for row in self.board ])
                    raise Exception()
        pass

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
            newState.aliveUnits = newState.aliveUnits.copy()
            newState.aliveUnits[self.iActive] = newState.aliveUnits[self.iActive].copy()
            newState.checkConsistency()
            self.checkConsistency()

            # mover is always a new-reference
            moverTile = newState.getBoardField(decision.frm)
            mover = newState.aliveUnits[self.iActive][moverTile.index].copy()
            newState.aliveUnits[self.iActive][moverTile.index] = mover
            mover.turnMoved = newState.turnId

            newState.setBoardField(decision.frm, EMPTY_FIELD)

            landingSpot = decision.to
            index = len(decision.moves)
            while True:
                movedTile = newState.getBoardField(landingSpot)
                newState.setBoardField(landingSpot, moverTile)
                newState.aliveUnits[self.iActive][moverTile.index].position = landingSpot
                if isEmpty(movedTile):
                    break
                index -= 1
                assert movedTile.team == self.iActive
                moverTile = movedTile
                newState.aliveUnits[self.iActive][moverTile.index] = newState.aliveUnits[self.iActive][moverTile.index].copy()
                landingSpot = getNeighbour( landingSpot, decision.moves[index].inverse() )

            #print(decision.frm, 'to', decision.to)
            #print("newState.board is", [ [ () if char is None else (char.cid, char.position) for char in row ] for row in newState.board ])

            return ( newState, Step(
                            typ='move', frm=decision.frm, to=decision.to, target=(self.getBoardFieldDeref(decision.frm).cid), 
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
        newState.aliveUnits = [ row.copy() for row in self.aliveUnits ]
        newState.players = newState.players.copy()
        newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
        newState.players[self.iActive].actionDeck = newState.players[self.iActive].actionDeck.copy()
        newState.players[self.iActive].actions = newState.players[self.iActive].actions.copy()

        newState.players[self.iActive].discard(decision.card)

        heroIndex = newState.getBoardField(decision.subjectPos).index
        assert newState.getBoardField(decision.subjectPos).team == self.iActive
        hero = newState.aliveUnits[self.iActive][heroIndex] = newState.aliveUnits[self.iActive][heroIndex].copy()
        newState.checkConsistency()
        if decision.card == ActionCard.DEFENSE:
            newShieldHP = hero.buff()
            return (newState, Step(typ='def', cardLost='defense', subject=decision.subjectPos, temporary=newShieldHP, permanent=50))
        else:
            hero.turnAttacked = newState.turnId
            setLife = 0
            victimIndex = newState.getBoardField(decision.objectPos).index
            assert newState.getBoardField(decision.objectPos).team == 1 - self.iActive
            victim = newState.aliveUnits[1 - self.iActive][victimIndex] = newState.aliveUnits[1 - self.iActive][victimIndex].copy()
            if decision.card == ActionCard.SOFTATK:
                lostLife = victim.takeDmg(False, hero.getAtk(False, newState.turnId))
                ret = Step(typ='atk', cardLost='softAtk', subject=decision.subjectPos, object=decision.objectPos, setLife=victim.HP, lostLife=lostLife)
            elif decision.card == ActionCard.HARDATK:
                lostLife = victim.takeDmg(True, hero.getAtk(True, newState.turnId))
                ret = Step(typ='atk', cardLost='hardAtk', subject=decision.subjectPos, object=decision.objectPos, setLife=victim.HP, lostLife=lostLife)
            else:
                raise AssertionError('decision.card is neither hard, soft, nor defense')
            if victim.HP <= 0:
                newState.board = [ row.copy() for row in self.board ]
                newState.nbAliveUnits = self.nbAliveUnits.copy()
                newState.nbAliveUnits[ 1 - self.iActive ] -= 1
                ret.kwargs['delete'] = True
                newState.setBoardField( victim.position, EMPTY_FIELD)
                newState.aliveUnits[ 1 - self.iActive ][ victim.teampos ] = DEAD_UNIT
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
                        ret.aliveUnits = [ row.copy() for row in ret.aliveUnits ]
                        dirty = True
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
            (pos, secondPass, movs) = stack.pop()
            if not secondPass:
                if movs: # do not select the empty list, aka "staying in place"
                    ret.append( (movs, pos) )
                if len(movs) < character.mov:
                    stack.append( (pos, True, movs) )
                    boardOfBools[pos[0]][pos[1]] = True
                    neighbours = []
                    if pos[1] != 0:
                        neighbours.append( (Direction.LEFT, getNeighbour(pos, Direction.LEFT)) )
                    if pos[1] != FULL_BOARD_WIDTH-1:
                        neighbours.append( (Direction.RIGHT, getNeighbour(pos, Direction.RIGHT)) )
                    neighbours.append( ((Direction.DOWN if pos[0] == 0 else Direction.UP), (1 - pos[0], pos[1])) )
                    for (direction, position) in neighbours:
                        if boardOfBools[position[0]][position[1]]:
                            continue
                        resident = self.getBoardField(position)
                        if not isEmpty(resident) and resident.team != character.team:
                            continue
                        stack.append( (position, False, movs + [ direction ]) )
            else: # second pass
                boardOfBools[pos[0]][pos[1]] = False
        return ret

    def allAttacks(self):
        assert self.timestep == Timestep.ABILITYCHOSEN
        ret = {}
#        print(f' iActive : { self.iActive }, units : { [unit.name for unit in self.aliveUnits[ self.iActive ] ] }')
        for iChar, character in enumerate( self.aliveUnits[ self.iActive ] ):
            if not isDead(character):
                ret[ iChar ] = []
                if character.arcAtk:
                    for row in range(2):
                        deltaRow = abs( character.position[0] - row )
                        for col in range( max(character.position[1] - character.rng + deltaRow, 0), min(character.position[1] + character.rng - deltaRow + 1, FULL_BOARD_WIDTH) ):
                            tile = self.board[row][col]
                            if not isEmpty(tile) and tile.team != self.iActive:
                                ret[ iChar ].append( tile.index )
                else:
                    row = character.position[0]
                    tile = self.board[1-row][character.position[1]]
                    if not isEmpty(tile) and tile.team != self.iActive:
                        ret[ iChar ].append( tile.index )
                    for col in range( character.position[1] - 1, max(character.position[1] - character.rng - 1, -1), -1 ):
                        tile = self.board[row][col]
                        if not isEmpty(tile):
                            if tile.team != self.iActive:
                                ret[ iChar ].append( tile.index )
                            break
                        if col != character.position[1] - character.rng:
                            tile = self.board[row][1-col]
                            if not isEmpty(tile) and tile.team != self.iActive:
                                ret[ iChar ].append( tile.index )
                    for col in range( character.position[1] + 1, min(character.position[1] + character.rng + 1, FULL_BOARD_WIDTH), +1 ):
                        tile = self.board[row][col]
                        if not isEmpty(tile):
                            if tile.team != self.iActive:
                                ret[ iChar ].append( tile.index )
                            break
                        if col != character.position[1] - character.rng:
                            tile = self.board[row][1-col]
                            if not isEmpty(tile) and tile.team != self.iActive:
                                ret[ iChar ].append( tile.index )
                if ret[iChar] == []:
                    del ret[iChar]
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
            "board": [ [ (tile.serialize() if not isEmpty(tile) else None ) for tile in row ] for row in self.board ],
            "aliveUnits" : [ [ (char.serialize() if not isDead(char) else None ) for char in units ] for units in self.aliveUnits ]
        }
