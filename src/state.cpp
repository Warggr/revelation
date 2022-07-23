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
#include <memory>
#include <tkDecls.h>
#define REPEAT(x) x, x, x, x

State::State(std::vector<std::vector<BoardTile>> board, std::vector<std::vector<character>> units, std::vector<Player> players, Deck<Faction> resDeck, Timestep timestep, int turnID) : resDeck(resDeck), board(board) {
    this->board = board;
    this->nbAliveUnits.reserve(ARMY_SIZE * 2);
    this->players = players;
    this->resDeck = resDeck;
    this->iActive = 0;
    this->timestep = timestep;
    this->units = units;
    this->turnID = turnID;
}

State State::createStart(Team teams[2]) {
    std::vector<std::vector<character>> alive;
    for(std::size_t i = 0; i < 2; i++ ) {
        for(std::size_t j = 0; j < teams[i].characters.size(); j++) {
            for(std::size_t k = 0; k < teams[i].characters[j].size(); k++) {
                character ch = teams[i].characters[j].at(k);
                ch.teampos = alive[i].size();
                alive[i].push_back(ch);
                ch.pos = position( j, k + 1 + HALF_BOARD_WIDTH);
                ch.team = i;
            }
        }
    }

    for(std::size_t i = 0; i < alive.size(); i++) {
        auto sortLambda = [] (character const& one, character const& two) -> bool{
            return one.maxAtk > two.maxAtk;
        };

        std::sort(alive[i].begin(), alive[i].end(), sortLambda);
        std::vector<character> characters = alive[i];

        for(std::size_t j = 0; j < characters.size(); j++) {
            characters[j].teampos = j;
            board[characters[j].pos.row][characters[j].pos.column] = BoardTile(i, j);
        }
    }

    for(std::size_t i = 0; i < players.size(); i++) {
        for(std::size_t j = 0; j < 2; j++) {
            this->players[j].drawAction();
        }
    }

    std::initializer_list<Faction> startingResources= { REPEAT(NONE), REPEAT(BLOOD), REPEAT(MERCURY), REPEAT(HORROR), REPEAT(SPECTRUM), REPEAT(ETHER), ETHER};
    Deck<Faction> resDeck = Deck<Faction>::create(startingResources);
    return State(board, alive, players, resDeck, BEGIN, 0);
}

BoardTile* State::getBoardField(position coords) {
    return &board[coords.row][coords.column];
}

character* State::getBoardFieldDeref(position coords) {
    BoardTile* ptr = this->getBoardField(coords);
    if(ptr == NULL)
        return nullptr;
    else
        return &this->units[ptr->team][ptr->index];
}

void State::setBoardField(position coords, BoardTile* value) {
    BoardTile tmp = BoardTile(value->team, value->index);
    board[coords.row][coords.column] = tmp;
}

void State::setBoardFieldDeref(position coords, BoardTile* value) {
    this->setBoardField(coords, value);
    if(&value != NULL)
        this->units[value->team][value->index].pos = coords;
}

bool State::isFinished() {
    return this->nbAliveUnits[0] == 0 || this->nbAliveUnits[1] == 0;
}

std::tuple<State*, Step> State::stepDraw(ActionOrResource decision) {
    assert(timestep == BEGIN);
    this->checkConsistency();
    State *newState = new State(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    newState->timestep = DISCARDED;
    //newState.players = newState.players.copy()
    //newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
    newState->checkConsistency();
    std::variant<ActionCard, Faction> cardDrawn;
    if(decision == ACTION) {
        cardDrawn = newState->players[iActive].drawAction();
        return { newState, StepTwo("draw", "action", cardDrawn, newState->players[iActive].actionDeck.sizeconfig())};
    } else {
        cardDrawn = newState->players[iActive].drawResource(newState->resDeck);
        return { newState, StepTwo("draw", "resource", cardDrawn, newState->resDeck.sizeconfig())};
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

position State::getNeighbour(position pos, Direction dir) {
    if(dir == UP)
        return position(pos.row - 1, pos.column);
    if(dir == DOWN)
        return position(pos.row + 1, pos.column);
    if(dir == LEFT)
        return position(pos.row, pos.column - 1);
    if(dir == RIGHT)
        return position(pos.row, pos.column + 1);
    else
        std::invalid_argument("Invalid direction!");
}

std::tuple<State*, Step> State::stepMov(MoveDecision decision) {
    this->checkConsistency();
    State *newState = new State(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    newState->checkConsistency();
    if(this->timestep == DISCARDED)
        newState->timestep = MOVEDfirst;
    else if(this->timestep == MOVEDfirst)
        newState->timestep = MOVEDlast;
    else
        throw std::invalid_argument(("Invalid timestep"));

    if(&decision == NULL) {
        newState->timestep = MOVEDlast;
        return { newState, StepOne("pass", "Did not move" ) };
    } else {
        newState->checkConsistency();
        this->checkConsistency();

        BoardTile* moverTile = newState->getBoardField(decision.from);
        character mover = newState->units[this->iActive][moverTile->index];
        newState->units[this->iActive][moverTile->index] = mover;
        mover.turnMoved = newState->turnID;

        newState->setBoardField(decision.from, NULL);

        position landingSpot = decision.to;
        int size = decision.moves.size();
        while(1) {
            BoardTile* movedTile = newState->getBoardField(landingSpot);
            newState->setBoardField(landingSpot, moverTile);
            newState->units[this->iActive][moverTile->index].pos = landingSpot;

            if(&movedTile == NULL)
                break;
            size--;

            assert(movedTile->team == this->iActive);
            moverTile = movedTile;
            landingSpot = this->getNeighbour(landingSpot, Direction(3 - decision.moves[size]));
        }
        newState->checkConsistency();

        return { newState, StepThree("move", decision.from, decision.to, this->getBoardFieldDeref(decision.from)->uid, decision.moves, size )};
    }
}

std::tuple<State*, Step> State::stepAbil(const AbilityDecision& decision) {
    this->timestep = MOVEDlast;
    State *newState = new State(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    if(decision.type == "pass") {
        newState->timestep = ABILITYCHOSEN;
        return { newState, StepOne("pass", "Abilities not implemented yet") };
    } else {
        std::logic_error("Not implemented yet");
    }
}

std::tuple<State *, Step> State::stepAct(ActionDecision decision) {
    assert(this->timestep == ABILITYCHOSEN);
    State* newState = this;
    newState->timestep = ACTED;
    if(&decision == NULL) {
        return { newState, StepOne("pass", "No action chosen") };
    }

    newState->players[this->iActive].discard(decision.card);
    int heroIndex = newState->getBoardField((decision.subjectPos))->index;
    assert(newState->getBoardField(decision.subjectPos)->team == this->iActive);
    character hero = newState->units[this->iActive][heroIndex];
    newState->checkConsistency();
    if(decision.card == DEFENSE) {
        short newShieldHP = hero.buff();
        //TODO: cardLost ist ein string?
        return std::make_tuple(newState, Step("def", ));
    } else {
        hero.turnAttacked = newState->turnID;
        int setLife = 0;
        int victimIndex = newState->getBoardField(decision.objectPos)->index;

        assert(newState->getBoardField(decision.subjectPos)->team == 1 - this->iActive);

        character victim = newState->units[1- this->iActive][victimIndex];
        if(decision.card == SOFTATK) {

        } else if(decision.card == HARDATK) {

        } else {
            std::invalid_argument("decision.card is neither hard, soft, nor defense");
        }

        if(victim.HP <= 0) {
            // TODO: units (aliveUnits) character array? -1 für pointer?
            // newState->units[1 - this->iActive] -= 1;
        }
    }

    delete newState;
}

std::tuple<State, Step> State::beginTurn() {
    this->checkConsistency();
    assert(this->timestep == ACTED);

    State *ret = new State(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    ret->iActive--;
    ret->timestep = BEGIN;
    bool dirty = false;

    for(int i = 0; i < ret->units[ret->iActive].size(); i++) {
        character unit = ret->units[ret->iActive][i];
        if(&unit != NULL) {
            std::tuple<State, Step> clone = unit.beginTurn();
            //TODO: tuple ?? wie vergleicht man die?
            if(std::tie(clone.)) {

            }
        }
    }
}

std::tuple<State *, Step> State::advance(Agent agent) {
    if(this->timestep == ACTED)
        return this->beginTurn();
    else if(this->timestep == BEGIN) {
        ActionOrResource decision = agent.getDrawAction();
        return this->stepDraw(decision);
    }
    else if(this->timestep == DISCARDED or this->timestep == MOVEDfirst) {
        MoveDecision decision = agent.getMovement();
        return this->stepMov(decision);
    }
    else if(this->timestep == MOVEDlast) {
        AbilityDecision decision = agent.getAbility();
        return this->stepAbil(decision);
    }
    if(this->timestep == ABILITYCHOSEN) {
        ActionDecision decision = agent.getAction();
        return this->stepAct(decision);
    }
}

json State::to_json(nlohmann::basic_json<> &j, const State &state) {
    j = json {{"resourceDeck", resDeck.sizeconfig()}};
    j.push_back("players");
    for(int i = 0; i < this->players.size(); i++) {
        j.at("players").insert(j.at("characters").begin(), players[i].to_json());
    }
    j.push_back("board");
    for(int i = 0; i < this->board.size(); i++) {
        j.at("players").insert(j.at("characters").begin(), board[i].to_json());
    }
    return j;
}

std::tuple<State *, Step> State::beginTurn() {
    this->checkConsistency();
    assert(this->timestep == ACTED);
    State *ret = new State(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    ret->iActive = 1 - ret->iActive;
    ret->timestep = BEGIN;
    bool dirty = false;
    for(int i = 0; i < ret->units[ret->iActive].size(); i++) {
        character unit = ret->units[ret->iActive][i];
        if( /*TODO */ ) {
                character clone = unit.beginTurn();
                if(clone.s_uid != unit.s_uid) {
                    if(!dirty) {
                        dirty = true;
                    }
                    ret->units[ret->iActive][i] = clone;
                }
        }
    }
    ret->checkConsistency();
    return std::make_tuple(ret, Step("beginturn"));
}

int State::allMovementsForCharacter(character hero) {
    if(this->timestep == MOVEDfirst && hero.turnMoved == this->turnID)
        return;
    std::vector<character> ret;
    std::vector<std::tuple<position, bool, std::vector<character>>> stack;

    while(!stack.empty()) {
        std::tuple<position, bool, std::vector<character>> val = stack.back();
        stack.pop_back();

        if(!std::get<1>(val)) {
            if()
        }
    }


}

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

