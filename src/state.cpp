#include "team.hpp"
#include "constants.hpp"
#include "agent.hpp"
#include "state.hpp"
#include "step.hpp"
#include <cassert>
#include <memory>
//#include <tkDecls.h>
#define REPEAT(x) x, x, x, x

State::State(Board board, std::array<std::array<character*, ARMY_SIZE>, 2> units, std::array<Player, 2> players,
             Deck<Faction> resDeck, Timestep timestep, int turnID) :
        board(board), resDeck(std::move(resDeck)), players(std::move(players)) {
    this->nbAliveUnits.reserve(ARMY_SIZE * 2);
    this->iActive = 0;
    this->timestep = timestep;
    this->units = units;
    this->turnID = turnID;
}

State State::createStart(std::array<Team, 2> teams) {
    std::array<std::array<character*, ARMY_SIZE>, 2> alive;
    for(int i = 0; i < 2; i++ ) {
        for(int j = 0; j < 2; j++) {
            for(int k = 0; k < ARMY_WIDTH; k++) {
                auto* ch = new character( teams[i].characters[j].at(k) );
                ch->teampos = alive[i].size();
                alive[i][k] = ch;
                ch->pos = position( j, k + 1 + HALF_BOARD_WIDTH);
                ch->team = i;
            }
        }
    }

    Board board;
    for(std::size_t i = 0; i < alive.size(); i++) {
        auto sortLambda = [] (character const* one, character const* two) -> bool{
            return one->maxAtk > two->maxAtk;
        };

        std::sort(alive[i].begin(), alive[i].end(), sortLambda);

        for(int j = 0; j < ARMY_WIDTH; j++) {
            character* ch = alive[i][j];
            ch->teampos = j;
            board[ch->pos.row][ch->pos.column] = BoardTile(i, j);
        }
    }

    std::array<Player, 2> players;
    for(int i = 0; i < 2; i++) {
        for(auto & player : players) {
            player.drawAction();
        }
    }

    Deck<Faction> resDeck = Deck<Faction>::create({ REPEAT(BLOOD), REPEAT(MERCURY), REPEAT(HORROR), REPEAT(SPECTRUM), ETHER });
    return State(board, alive, players, resDeck, BEGIN, 0);
}

const BoardTile& State::getBoardField(position coords) const {
    return board[coords.row][coords.column];
}

character* State::getBoardFieldDeref(position coords) const {
    const BoardTile& ptr = this->getBoardField(coords);
    return this->units[ptr.team][ptr.index];
}

void State::setBoardField(position coords, BoardTile value) {
    board[coords.row][coords.column] = value;
}

void State::setBoardFieldDeref(position coords, BoardTile value) {
    this->setBoardField(coords, value);
    if(not BoardTile::isEmpty(value))
        this->units[value.team][value.index]->pos = coords;
}

bool State::isFinished() const {
    return this->nbAliveUnits[0] == 0 || this->nbAliveUnits[1] == 0;
}

std::tuple<State, uptr<DrawStep>> State::stepDraw(ActionOrResource decision) const {
    assert(timestep == BEGIN);
    this->checkConsistency();
    State newState(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    newState.timestep = DISCARDED;
    //newState.players = newState.players.copy()
    //newState.players[self.iActive] = copy.copy(newState.players[self.iActive])
    newState.checkConsistency();
    std::variant<ActionCard, Faction> cardDrawn;
    if(decision == ACTION) {
        cardDrawn = newState.players[iActive].drawAction();
        return { newState, std::make_unique<DrawStep>(cardDrawn, newState.players[iActive].actionDeck.sizeconfig()) };
    } else {
        cardDrawn = newState.players[iActive].drawResource(newState.resDeck);
        return { newState, std::make_unique<DrawStep>( cardDrawn, newState.resDeck.sizeconfig())};
    }
}

void State::checkConsistency() const {
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

std::tuple<State, uptr<MoveStep>> State::stepMov(MoveDecision decision) const {
    this->checkConsistency();
    State newState(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    newState.checkConsistency();
    if(this->timestep == DISCARDED)
        newState.timestep = MOVEDfirst;
    else if(this->timestep == MOVEDfirst)
        newState.timestep = MOVEDlast;
    else
        throw std::invalid_argument(("Invalid timestep"));

    if(MoveDecision::isPass(decision)) {
        newState.timestep = MOVEDlast;
        uptr<MoveStep> u = std::make_unique<MoveStep>(MoveStep::pass());
        return std::make_tuple( newState, std::move(u) );
    } else {
        newState.checkConsistency();
        this->checkConsistency();

        BoardTile moverTile = newState.getBoardField(decision.from);
        character* mover = newState.units[this->iActive][moverTile.index];
        newState.units[this->iActive][moverTile.index] = mover;
        mover->turnMoved = newState.turnID;

        newState.setBoardFieldDeref(decision.from, BoardTile::empty());

        position landingSpot = decision.to;
        unsigned size = decision.moves.size();
        while(true) {
            BoardTile movedTile = newState.getBoardField(landingSpot);
            newState.setBoardFieldDeref(landingSpot, moverTile);

            if(BoardTile::isEmpty(movedTile))
                break;
            size--;

            assert(movedTile.team == this->iActive);
            moverTile = movedTile;
            landingSpot = getNeighbour(landingSpot, Direction(3 - decision.moves[size]));
        }
        newState.checkConsistency();

        uptr<MoveStep> u = std::make_unique<MoveStep>(decision.from, decision.to, this->getBoardFieldDeref(decision.from)->uid, decision.moves, size );
        return { newState, std::move(u) };
    }
}

std::tuple<State, uptr<AbilityStep>> State::stepAbil(const AbilityDecision& decision) const {
    assert(this->timestep == MOVEDlast);
    State newState(this->board, this->units, this->players, this->resDeck, this->timestep, this->turnID);
    assert(decision.isPass());
    newState.timestep = ABILITYCHOSEN;
    return { newState, std::make_unique<AbilityStep>() };
}

std::tuple<State, uptr<ActionStep>> State::stepAct(ActionDecision decision) const {
    assert(this->timestep == ABILITYCHOSEN);
    State newState(*this);
    newState.timestep = ACTED;
    if(decision.isPass()) {
        return { newState, std::make_unique<ActionStep>(ActionStep::pass()) };
    }

    newState.players[this->iActive].discard(decision.card);
    int heroIndex = newState.getBoardField((decision.subjectPos)).index;
    assert(newState.getBoardField(decision.subjectPos).team == this->iActive);
    character* hero = newState.units[this->iActive][heroIndex];
    newState.checkConsistency();
    if(decision.card == DEFENSE) {
        short newShieldHP = hero->buff();
        return std::make_tuple(newState, std::make_unique<ActionStep>( decision.card, decision.subjectPos, decision.objectPos, newShieldHP, 50 ));
    } else {
        hero->turnAttacked = newState.turnID;
        int setLife = 0;
        int victimIndex = newState.getBoardField(decision.objectPos).index;
        uptr<ActionStep> step;

        assert(newState.getBoardField(decision.subjectPos).team == 1 - this->iActive);

        character* victim = newState.units[1- this->iActive][victimIndex];
        if(decision.card == SOFTATK) {
            setLife = victim->takeDmg(false, hero->softAtk);
            step = std::make_unique<ActionStep>(decision.card, decision.subjectPos, decision.objectPos, setLife, hero->softAtk );
        } else if(decision.card == HARDATK) {
            setLife = victim->takeDmg(true, hero->hardAtk);
            step = std::make_unique<ActionStep>(decision.card, decision.subjectPos, decision.objectPos, setLife, hero->hardAtk );
        } else {
            throw std::invalid_argument("decision.card is neither hard, soft, nor defense");
        }

        if(victim->HP <= 0) {
            newState.setBoardField(decision.objectPos, BoardTile::empty());
            newState.units[1 - this->iActive][ victim->teampos ] = DEAD_UNIT;
            newState.nbAliveUnits[1 - this->iActive] -= 1;
            step->del = true;
        }
        newState.checkConsistency();
        return std::make_tuple(newState, std::move(step));
    }
}

std::tuple<State, uptr<BeginStep>> State::beginTurn() const {
    this->checkConsistency();
    assert(this->timestep == ACTED);
    State ret(*this);
    ret.iActive = 1 - ret.iActive;
    ret.timestep = BEGIN;
    //bool dirty = false;
    for(int i = 0; i < ARMY_SIZE; i++) {
        character* unit = ret.units[ret.iActive][i];
        if( not isDead(unit) ) {
            character* clone = unit->beginTurn();
            if(clone->uid != unit->uid) {
                //dirty = true;
                ret.units[ret.iActive][i] = clone;
            }
        }
    }
    // TODO invalidate and clone the board if dirty
    ret.checkConsistency();
    return std::make_tuple(ret, std::make_unique<BeginStep>());
}

std::tuple<State, uptr<Step>> State::advance(Agent& agent) const {
    switch(this->timestep){
    case ACTED:
        return this->beginTurn();
    case BEGIN: {
        ActionOrResource decision = agent.getDrawAction(*this);
        return this->stepDraw(decision);
    }
    case DISCARDED:
    case MOVEDfirst: {
        MoveDecision decision = agent.getMovement(*this, (timestep==DISCARDED ? 0 : 1));
        return this->stepMov(decision);
    }
    case MOVEDlast: {
        AbilityDecision decision = agent.getAbility(*this);
        return this->stepAbil(decision);
    }
    case ABILITYCHOSEN: {
        ActionDecision decision = agent.getAction(*this);
        return this->stepAct(decision);
    }
    case DREW: //currently DREW never happens
    default: //shouldn't happen either
        return std::make_tuple<State, uptr<Step>>({}, {});
    }
}

json State::to_json(nlohmann::basic_json<> &j) const {
    /*j = json {{"resourceDeck", resDeck.sizeconfig()}};
    j.push_back("players");
    for(int i = 0; i < this->players.size(); i++) {
        j.at("players").insert(j.at("characters").begin(), players[i].to_json(j));
    }
    j.push_back("board");
    for(int i = 0; i < this->board.size(); i++) {
        j.at("players").insert(j.at("characters").begin(), board[i].to_json());
    }
    return j;*/
    return j; //TODO
}

std::vector<MoveDecision> State::allMovementsForCharacter(character hero) const {
    if(this->timestep == MOVEDfirst && hero.turnMoved == this->turnID)
        return {};
    std::vector<character> ret;
    std::vector<std::tuple<position, bool, std::vector<character>>> stack;

    while(!stack.empty()) {
        std::tuple<position, bool, std::vector<character>> val = stack.back();
        stack.pop_back();

        if(!std::get<1>(val)) {
            //TODO
        }
    }
    return {}; //TODO
}

/*
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
*/
