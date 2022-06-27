/*from player import Player
from team import Team
from character import Character
from constants import Timestep, FULL_BOARD_WIDTH, HALF_BOARD_WIDTH, Faction
from card import Deck, ActionCard
from serialize import Serializable
from agent import ActionOrResource*/


struct Step /*: public Serializable */ {

};

/*class Step(Serializable):
    def serialize(self):
        return { 'action' : self.type, **self.kwargs }*/


State State::createStart(Team teams[2]) {
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

        startingResources = sum([[x] * 4 for x in Faction.allFactions()], []) + [ Faction.ETHER ]
        resDeck = Deck.create(startingResources)

        return State(board, alive, players, resDeck, Timestep.BEGIN)
}

    def copy(self):
        #ret = State(self.board, self.aliveUnits, self.players, self.resDeck, self.timestep)
        #ret.iActive = self.iActive
        #return ret
        return copy.copy(self)

    def getBoardField(self, coords):
        return self.board[ coords[0] ][ coords[1] ]
    def setBoardField(self, coords, value):
        self.board[ coords[0] ][ coords[1] ] = value
        if(value):
            value.position = coords
            self.aliveUnits[value.team][value.teampos] = value

    def isFinished(self):
        return self.nbAliveUnits[0] == 0 or self.nbAliveUnits[1] == 0

    def stepDraw(self, decision : ActionOrResource):
        assert self.timestep == Timestep.BEGIN
        newState = self.copy()
        newState.timestep = Timestep.DISCARDED
        newState.players = newState.players.copy()
        newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
        if decision == ActionOrResource.ACTION:
            newState.players[self.iActive].actions = newState.players[self.iActive].actions.copy()
            newState.players[self.iActive].actionDeck = newState.players[self.iActive].actionDeck.copy()
            cardDrawn = newState.players[self.iActive].drawAction()
            return ( newState, Step( typ='draw', clss='action', value=cardDrawn ) )
        else:
            newState.resDeck = newState.resDeck.copy()
            newState.players[self.iActive].resources = newState.players[self.iActive].resources.copy()
            cardDrawn = newState.players[self.iActive].drawResource(newState.resDeck)
            return ( newState, Step( typ='draw', clss='resource', value=cardDrawn ) )

    def checkConsistency(self):
        for team in self.aliveUnits:
            for char in team:
                if char is not None and self.getBoardField( char.position ) is not char:
                    print('!Error:', self.getBoardField( char.position ), '@', char.position, 'is not', char)
                    print( [ [ (char.cid, char.position) for char in row ] for row in self.aliveUnits ] )
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
            newState.board = [ row[:] for row in self.board ]
            newState.aliveUnits = [ row[:] for row in self.aliveUnits ]

            #print('Moving:')
            newState.checkConsistency()

            mover = newState.getBoardField(decision.frm)
            moved = newState.getBoardField(decision.to)
            moved = moved.copy() if moved else moved
            mover = mover.copy() if mover else mover
            newState.setBoardField(decision.frm, moved)
            newState.setBoardField(decision.to, mover)
            #print(decision.frm, 'to', decision.to)
            #print("newState.board is", [ [ () if char is None else (char.cid, char.position) for char in row ] for row in newState.board ])

            newState.checkConsistency()

            return ( newState, Step(
                            typ='move', frm=decision.frm, to=decision.to, target=(self.getBoardField(decision.frm).cid), isCOF=(self.getBoardField(decision.to) != None) 
                        )
            )

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
        newState.board = [ row[:] for row in self.board ]
        newState.aliveUnits = [ row[:] for row in self.aliveUnits ]
        if decision.card == ActionCard.DEFENSE:
            hero = decision.subject.copy()
            newState.setBoardField( hero.position, hero )
            (newHP, newTempHP) = hero.buff()
            return (newState, Step(typ='def', cardLost='defense', subject=decision.subject.position, temporary=newTempHP, permanent=newHP))
        else:
            setLife = 0
            victim = decision.object.copy()
            if decision.card == ActionCard.SOFTATK:
                lostLife = victim.takeDmg('soft', decision.subject.softAtk)
                ret = Step(typ='atk', cardLost='softAtk', subject=decision.subject.position, object=decision.object.position, setLife=victim.HP, lostLife=lostLife)
            elif decision.card == ActionCard.HARDATK:
                lostLife = victim.takeDmg('hard', decision.subject.hardAtk)
                ret = Step(typ='atk', cardLost='hardAtk', subject=decision.subject.position, object=decision.object.position, setLife=victim.HP, lostLife=lostLife)
            else:
                raise AssertionError('decision.card is neither hard, soft, nor defense')
            if victim.HP <= 0:
                newState.setBoardField( victim.position, None )
                newState.nbAliveUnits = self.nbAliveUnits[:]
                newState.nbAliveUnits[ 1 - self.iActive ] -= 1
                newState.aliveUnits = self.aliveUnits[:]
                newState.aliveUnits[ 1 - self.iActive ] = [ unit if unit is not None and unit.cid != victim.cid else None for unit in newState.aliveUnits[ 1 - self.iActive ] ]
                ret.kwargs['delete'] = True
            else:
                newState.setBoardField( victim.position, victim )
            return (newState, ret)

    def endTurn(self):
        assert self.timestep == Timestep.ACTED
        ret = self.copy()
        ret.iActive = 1 - ret.iActive
        ret.timestep = Timestep.BEGIN
        return (ret, Step(typ='pass', message='End of turn'))

    def allMovementsForCharacter(self, character : Character) -> list[tuple[int, int]]:
        ret = []
        for i in range( max(character.position[1] - character.mov, 0), min(character.position[1] + character.mov + 1, FULL_BOARD_WIDTH) ):
            ret.append( ( character.position[0], i ) )
        for i in range( max(character.position[1] - character.mov + 1, 0), min(character.position[1] + character.mov, FULL_BOARD_WIDTH) ):
            ret.append( ( 1 - character.position[0], i ) )
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
        if self.timestep == Timestep.BEGIN:
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
        elif self.timestep == Timestep.ACTED:
            return self.endTurn()
