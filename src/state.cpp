/*from player import Player
from team import Team
from character import Character
from constants import Timestep, FULL_BOARD_WIDTH, HALF_BOARD_WIDTH, Faction
from card import Deck, ActionCard
from serialize import Serializable
from agent import ActionOrResource*/

#include "team.hpp"
#include "constants.hpp"
#include "agent.hpp"
#include "state.hpp"
#include <assert.h>
#include "step.hpp"

State::State(std::vector<std::vector<character>> board, std::vector<std::vector<character>> units,
             std::vector<uint8_t> nbAliveUnits, std::vector<Player> players, Deck<ActionCard> resDeck, uint8_t iActive, Timestep timestep) : resDeck(resDeck) {
    this->board = board;
    this->nbAliveUnits = nbAliveUnits;
    this->players = players;
    this->resDeck = resDeck;
    this->iActive = 0;
    this->timestep = timestep;
    this->units = units;
}

template <typename T>
State State::createStart(team teams[2]) {
    std::vector<std::vector<character>> alive;
    for(std::size_t i = 0; i < 2; i++ ) {
        for(uint8_t j = 0; j < teams[i].characters.size(); j++) {
            teams[i].characters[j].teampos = alive[i].size();
            alive[i].push_back(teams[i].characters[j]);
            board[ j ][ j + 1 + HALF_BOARD_WIDTH*i ] = teams[i].characters[j];
            teams[i].characters[j].pos = position( i != 1, j + 1);
            //teams[i].characters[j].team = i;
        }
    }

    for(std::size_t i = 0; i < players.size(); i++) {
        for(std::size_t j = 0; j < 2; j++) {
            players[j].drawAction();
        }
    }
    /*Deck<ActionCard> deck;
    Faction allFactions[] = {BLOOD, MERCURY, HORROR, SPECTRUM};
    std::vector<T> cards;
    for(std::size_t i = 0; i < 4; i++) {
        cards.push_back();
    }
        startingResources = sum([[x] * 4 for x in Faction.allFactions()], []) + [ Faction.ETHER ]
        resDeck = Deck.create(startingResources);*/
    //return new State(board, alive, players, deck, BEGIN, 0);
}

character State::getBoardField(position coords) {
    return board[coords.row][coords.column];
}

void State::setBoardField(position coords, character value) {
    board[coords.row][coords.column] = value;
    if(&value != NULL) {
        value.pos = coords;
        units[value.team][value.teampos] = value;
    }
}

bool State::isFinished() {
    return nbAliveUnits[0] == 0 || nbAliveUnits[1] == 0;
}

std::tuple<State*, Step> State::stepDraw(ActionOrResource decision) {
    assert(timestep == BEGIN);
    this->checkConsistency();
    State *newState = this;
    newState->timestep = DISCARDED;
    //newState.players = newState.players.copy()
    //newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
    newState->checkConsistency();
    ActionCard cardDrawn;
    if(decision == ACTION) {
        cardDrawn = newState->players[iActive].drawAction();
        return { newState, Step("draw", "action", cardDrawn, newState->players[iActive].actionDeck.size())};
    } else {
        cardDrawn = newState->players[iActive].drawResource(newState->resDeck);
        return { newState, Step("draw", "resource", cardDrawn, newState->resDeck.size())};
    }
}

void State::checkConsistency() {
    /*for team in self.aliveUnits:
    for char in team:
    if char is not None and self.getBoardField( char.position ) is not char:
    print('!Error:', self.getBoardField( char.position ), '@', char.position, 'is not', char)
    print( [ [ (char.cid, char.position) for char in row ] for row in self.aliveUnits ] )
    print([ [ () if char is None else (char.cid, char.position) for char in row ] for row in self.board ])
    raise Exception()}
    def checkConsistency(self)*/
    //TODO
}

std::tuple<State*, Step> State::stepMov(MoveDecision decision) {
    this->checkConsistency();
    State *newState = this;
    newState->checkConsistency();
    if(this->timestep == DISCARDED)
        newState->timestep = MOVEDfirst;
    else if(this->timestep == MOVEDfirst)
        newState->timestep = MOVEDlast;
    else
        throw std::invalid_argument(("Invalid timestep"));

    if(&decision == NULL) {
        newState->timestep = MOVEDlast;
        return { newState, Step("pass", "Did not move" ) };
    } else {
        for(int i = 0; i < this->board.size(); i++)
            newState->board[i] = this->board[i];
        for(int i = 0; i < this->units.size(); i++)
            newState->units[i] = this->units[i];

        newState->checkConsistency();

        character mover = newState->getBoardField(decision.from);
        character moved = newState->getBoardField(decision.to);
        newState->setBoardField(decision.from, moved);
        newState->setBoardField(decision.to, mover);

        newState->checkConsistency();

        return { newState, Step("move", decision.from, decision.to, this->getBoardField(decision.from).s_uid, this->getBoardField(decision.to)) };
    }
}

std::tuple<State*, Step> State::stepAbil(AbilityDecision decision) {
    this->timestep = MOVEDlast;
    State* newState = this;
    if(decision.type == "pass") {
        newState->timestep = ABILITYCHOSEN;
        return { newState, Step("pass", "Abilities not implemented yet") };
    } else {
        std::logic_error("Not implemented yet");
    }
}

    def stepAct(self, decision : 'ActionDecision'):
         self.timestep == Timestep.ABILITYCHOSEN
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
         self.timestep == Timestep.ACTED
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
         self.timestep == Timestep.ABILITYCHOSEN
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
