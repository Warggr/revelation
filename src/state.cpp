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
            if(clone != unit) {
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

std::vector<MoveDecision> State::allMovementsForCharacter(character hero) const {
    if(this->timestep == MOVEDfirst && hero.turnMoved == this->turnID)
        return {};
    std::vector<MoveDecision> ret;

    bool boardOfBools[FULL_BOARD_WIDTH][2]; //whether that field has already been passed in the current iteration or not
    for(uint i=0; i<2; i++) for(uint j=0; j<FULL_BOARD_WIDTH; j++) boardOfBools[i][j] = false;

    std::vector<std::tuple<position, bool, std::vector<Direction>>> stack;
    stack.emplace_back(hero.pos, false, std::vector<Direction>());

    while(!stack.empty()) {
        auto [ pos, secondPass, moves ] = stack.back();
        stack.pop_back();

        if(not secondPass) {
            if(not moves.empty()) //do not select the empty list, aka "staying in place"
                ret.emplace_back( hero.pos, pos, moves );
            if(moves.size() < hero.mov){
                stack.emplace_back( pos, true, moves );
                boardOfBools[pos.row][pos.column] = true;
                std::vector<std::tuple<Direction, position>> neighbours;
                if(pos.column != 1)
                    neighbours.emplace_back( Direction::LEFT, getNeighbour(pos, Direction::LEFT) );
                if(pos.column != FULL_BOARD_WIDTH - 1)
                    neighbours.emplace_back( Direction::RIGHT, getNeighbour(pos, Direction::RIGHT) );
                neighbours.emplace_back( (pos.row == 0 ? Direction::DOWN : Direction::UP), position(pos.row, pos.column) );
                for(auto& [ dir, pos ] : neighbours){
                    if(boardOfBools[pos.row][pos.column]) continue; //can't pass twice through the same field
                    const BoardTile& resident = this->getBoardField(pos);
                    if(not BoardTile::isEmpty(resident) and resident.team != hero.team) continue; //can't pass through an opponent
                    auto newMoves = moves;
                    newMoves.push_back(dir);
                    stack.emplace_back( pos, false, newMoves );
                }
            }
        } else { //second pass
            boardOfBools[pos.row][pos.column] = false;
        }
    }
    return ret;
}

std::map<const character*, std::vector<character*>> State::allAttacks() const {
    assert(timestep == Timestep::ABILITYCHOSEN);
    std::map<const character*, std::vector<character*>> ret;
    //print(f' iActive : { this->iActive }, units : { [unit.name for unit in this->units[ this->iActive ] ] }')
    for(uint iChar = 0; iChar < ARMY_SIZE; iChar++){
        const character* chr = this->units[this->iActive][iChar];
        if(isDead(chr)) continue;

        std::vector<character*> enemies;
        if(chr->usesArcAttack){
            for(int row = 0; row < 2; row++){
                int deltaRow = (chr->pos.row == row ? 0 : 1);
                int min = std::max(chr->pos.column - chr->rng + deltaRow, 0);
                int max = std::min(chr->pos.column + chr->rng - deltaRow, FULL_BOARD_WIDTH - 1);
                for(int col = min; col <= max; col++){
                    const BoardTile& tile = this->board[row][col];
                    if(not BoardTile::isEmpty(tile) and tile.team != this->iActive){
                        enemies.push_back( this->units[1-this->iActive][tile.index] );
                    }
                }
            }
        } else {
            auto row = chr->pos.row;
            const BoardTile& tile = this->board[1-row][chr->pos.column];
            if(not BoardTile::isEmpty(tile) and tile.team != this->iActive){
                enemies.push_back( this->units[1-this->iActive][tile.index] );
            }
            for(int col = chr->pos.column - 1; col >= 0 and col >= chr->pos.column - chr->rng; col--){
                const BoardTile& tile = this->board[row][col];
                if(not BoardTile::isEmpty(tile)){
                    if(tile.team != this->iActive)
                        enemies.push_back( this->units[1-this->iActive][tile.index] );
                    break;
                }
                if(col != chr->pos.column - chr->rng){
                    const BoardTile& tile = this->board[row][1-col];
                    if(not BoardTile::isEmpty(tile) and tile.team != this->iActive)
                        enemies.push_back( this->units[1-this->iActive][tile.index] );
                }
            }
            for(int col = chr->pos.column + 1; col < FULL_BOARD_WIDTH and col <= chr->pos.column + chr->rng; col++ ){
                const BoardTile& tile = this->board[row][col];
                if(not BoardTile::isEmpty(tile)){
                    if(tile.team != this->iActive)
                        enemies.push_back( this->units[1-this->iActive][tile.index] );
                    break;
                }
                if(col != chr->pos.column - chr->rng){
                    const BoardTile& tile = this->board[row][1-col];
                    if(not BoardTile::isEmpty(tile) and tile.team != this->iActive)
                        enemies.push_back( this->units[1-this->iActive][tile.index] );
                }
            }
        }
        if(!enemies.empty()){
            auto [ iter, wasInserted ] = ret.insert(std::make_pair<>( chr, enemies ));
            assert(wasInserted);
        }
    }
    return ret;
}
