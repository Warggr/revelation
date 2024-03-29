#include "state.hpp"
#include "step.hpp"
#include "setup/team.hpp"
#include "constants.hpp"
#include <cassert>
#include <memory>
#include <iostream>
#define REPEAT(x) x, x, x, x

std::ostream& operator<<(std::ostream& o, const position& pos){
    o << '[' << pos.column << ", " << pos.row << ']';
    return o;
}

std::ostream& operator<<(std::ostream& o, const BoardTile& tile){
    o << static_cast<int>(tile.team) << '[' << tile.index << ']';
    return o;
}

std::ostream& operator<<(std::ostream& o, const Board& board){
    o << "---BOARD---\n";
    for(uint row = 0; row < 2; row++){
        for(uint col = 0; col < FULL_BOARD_WIDTH; col++){
            const BoardTile& tile = board[row][col];
            if(BoardTile::isEmpty(tile)) o << "[   ]";
            else o << '[' << static_cast<int>(tile.team) << '.' << tile.index << ']';
        }
        o << '\n';
    }
    return o;
}

State& State::operator=(const State& copy){
    copy.checkConsistency();
    board = copy.board;
    nbAliveUnits = copy.nbAliveUnits;
    turnID = copy.turnID;
    unresolvedSpecialAbility = copy.unresolvedSpecialAbility;
    timestep = copy.timestep;
    iActive = copy.iActive;
    players = copy.players;
    for(int iSide = 0; iSide < 2; iSide++){
        for(int j = 0; j<ARMY_SIZE; j++){
            this->units[iSide][j] = copy.units[iSide][j].copy();
            //copying all the smart pointers
        }
    }
    this->checkConsistency();
    assert(*this == copy);
    return *this;
}

State State::createStart(const std::array<const Team*, 2>& teams, Generator generator) {
    State retVal(generator);
    retVal.turnID = 0;
    retVal.timestep = Timestep::BEGIN;
    retVal.iActive = 0;

    char character_uid = 'a';

    std::vector<NullableShared<Character>> units[2];
    for(int i = 0; i < 2; i++ ) {
        for(int j = 0; j < 2; j++) {
            for(int k = 0; k < ARMY_WIDTH; k++) {
                if(teams[i]->characters[j][k] != nullptr){ //0 indicates an empty field
                    NullableShared<Character> character_ptr = { *teams[i]->characters[j][k], character_uid++};
                    character_ptr->pos = position( j, k + 1 + i*HALF_BOARD_WIDTH);
                    character_ptr->team = i;
                    units[i].push_back(std::move(character_ptr));
                }
            }
        }
    }
    retVal.nbAliveUnits = { (short unsigned) units[0].size(), (short unsigned) units[1].size() };

    for(std::size_t i = 0; i < 2; i++) {
        auto sortLambda = [] (const NullableShared<Character>& one, const NullableShared<Character>& two) -> bool{
            return one->im.maxAtk > two->im.maxAtk;
        };

        std::sort(units[i].begin(), units[i].end(), sortLambda);

        for(uint j = 0; j < units[i].size(); j++) {
            auto& ch = units[i][j];
            ch->teampos = j;
            retVal.board[ch->pos.row][ch->pos.column] = BoardTile(i, j);
            retVal.units[i][j] = std::move(units[i][j]);
        }
        for(int j = units[i].size(); j < ARMY_SIZE; j++) assert( retVal.units[i][j].get() == DEAD_UNIT );
    }

    // Each player draws 2 cards
    for(int i = 0; i < 2; i++) {
        for(auto & player : retVal.players) {
            player.drawCard();
        }
    }

    return retVal;
}

const BoardTile& State::getBoardField(position coords) const {
    return board[coords.row][coords.column];
}

Character* State::getBoardFieldDeref(position coords) {
    const BoardTile& ptr = this->getBoardField(coords);
    return this->units[ptr.team][ptr.index].get();
}

const Character* State::getBoardFieldDeref(position coords) const {
    const BoardTile& ptr = this->getBoardField(coords);
    return this->units[ptr.team][ptr.index].get();
}

void State::setBoardField(position coords, BoardTile value) {
    board[coords.row][coords.column] = value;
}

void State::setBoardFieldDeref(position coords, BoardTile value) {
    this->setBoardField(coords, value);
    if(not BoardTile::isEmpty(value))
        this->units[value.team][value.index]->pos = coords;
}

unsigned short int State::getWinner() const {
    const auto nb1 = this->nbAliveUnits[0]; const auto nb2 = this->nbAliveUnits[1];
    if(nb1 != 0 and nb2 != 0) return 0;
    else if(nb1 != 0 and nb2 == 0) return 1;
    else if(nb1 == 0 and nb2 != 0) return 2;
    else if(nb1 == 0 and nb2 == 0) return 3;

    assert(false);
    return 0;
}

std::tuple<State, uptr<DrawStep>> State::stepDraw() const {
    assert(timestep == BEGIN);
    this->checkConsistency();
    State newState(*this);
    newState.timestep = DREW;
    //newState.players = newState.players.copy()
    //newState.players[this->iActive] = copy.copy(newState.players[this->iActive])
    newState.checkConsistency();
    std::variant<ActionCard, Faction> cardDrawn;
    cardDrawn = newState.players[iActive].drawCard();
    if(newState.players[iActive].getActions().size() + newState.players[iActive].getResourceCards().size() <= MAX_CARDS_IN_HAND )
        newState.timestep = DISCARDED;
        // no need to discard anything - so we skip discard step and pretend we have already discarded
    return { newState, std::make_unique<DrawStep>(cardDrawn, newState.players[iActive].deck.sizeconfig()) };
}

std::tuple<State, uptr<DiscardStep>> State::stepDiscard(DiscardDecision decision) const {
    assert(timestep == DREW);
    assert(players[this->iActive].getActions().size() + players[this->iActive].getResourceCards().size() > MAX_CARDS_IN_HAND);
    this->checkConsistency();
    State newState(*this);
    newState.timestep = DISCARDED;
    //newState.players = newState.players.copy()
    //newState.players[this->iActive] = copy.copy(newState.players[this->iActive])
    newState.checkConsistency();
    if(decision.discardedAction)
        newState.players[this->iActive].discardAction(decision.iCardDiscarded);
    else
        newState.players[this->iActive].discardResource(decision.iCardDiscarded);
    return { newState, std::make_unique<DiscardStep>() }; //TODO put correct step
}

void State::checkConsistency() const {
#ifndef NDEBUG
#ifdef ASSERTIONS_CHECK_CONSISTENCY
    if(not isInvalid(*this)){
        for(unsigned int i = 0; i < 2; i++){
            const Player &pl = this->players[i];
            auto decksize = pl.deck.sizeconfig();
            assert(std::get<0>(decksize) + std::get<1>(decksize) + pl.getActions().size() + pl.getResourceCards().size() == 10);
        }
    }
    for(unsigned int i = 0; i<2; i++){
        for(unsigned int j = 0; j<ARMY_SIZE; j++){
            const Character* ch = this->units[i][j].get();
            if(not isDead(ch)){
                const BoardTile& tile = this->getBoardField( ch->pos );
                if(tile.team != i or tile.index != j){
                    std::cout << "!Error: team" << i << '[' << j << "] is " << ch << " on " << ch->pos << " supposed to be tile " << tile << '\n';
                    std::cout << board;
                    for(int i = 0; i<2; i++){
                        for(int j = 0; j<ARMY_SIZE; j++){
                            if(isDead(units[i][j])) std::cout << " DEAD ";
                            else std::cout << units[i][j]->pos;
                        }
                        std::cout << '\n';
                    }
                    throw 1;
                }
            }
        }
    }
#endif
#endif
}

std::tuple<State, uptr<MoveStep>> State::stepMov(MoveDecision decision) const {
    this->checkConsistency();
    State newState(*this);
    newState.checkConsistency();
    if(this->timestep == DISCARDED)
        newState.timestep = MOVEDfirst;
    else if(this->timestep == MOVEDfirst)
        newState.timestep = MOVEDlast;
    else
        throw std::invalid_argument(("Invalid timestep"));

    if(decision.isPass()) {
        newState.timestep = MOVEDlast;
        uptr<MoveStep> u = std::make_unique<MoveStep>(MoveStep::pass());
        return std::make_tuple( newState, std::move(u) );
    } else {
        newState.checkConsistency();
        this->checkConsistency();

        BoardTile moverTile = newState.getBoardField(decision.from);
        assert(moverTile.team == this->iActive);
        NullableShared<Character> mover = newState.units[this->iActive][moverTile.index].copy();
        mover->turnMoved = newState.turnID;
        newState.units[this->iActive][moverTile.index] = std::move(mover);

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
    (void)decision;
    assert(this->timestep == MOVEDlast);
    State newState(*this);
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

    int heroIndex = newState.getBoardField((decision.subjectPos)).index;
    assert(newState.getBoardField(decision.subjectPos).team == this->iActive);
    Character* hero = newState.units[this->iActive][heroIndex].get();
    newState.checkConsistency();
    if(decision.card == SPECIALACTION){
        newState.unresolvedSpecialAbility = hero->getSpecialAction();
        return std::make_tuple(newState, std::make_unique<ActionStep>( decision.card, decision.subjectPos, decision.objectPos, 0, 0 ));
    }
    newState.players[this->iActive].discard(decision.card);
    if(decision.card == DEFENSE) {
        hero->buff();
        return std::make_tuple(newState, std::make_unique<ActionStep>( decision.subjectPos, decision.objectPos, hero->defShieldHP, 50 ));
    } else {
        hero->turnAttacked = newState.turnID;
        int victimIndex = newState.getBoardField(decision.objectPos).index;
        uptr<ActionStep> step;

        assert(newState.getBoardField(decision.objectPos).team == 1 - this->iActive);

        Character* victim = newState.units[1- this->iActive][victimIndex].get();
        if(decision.card == SOFTATK) {
            short lostHP = victim->takeDmg(false, hero->im.softAtk);
            step = std::make_unique<ActionStep>(decision.card, decision.subjectPos, decision.objectPos, victim->HP, lostHP );
        } else if(decision.card == HARDATK) {
            short lostHP = victim->takeDmg(true, hero->im.hardAtk);
            step = std::make_unique<ActionStep>(decision.card, decision.subjectPos, decision.objectPos, victim->HP, lostHP );
        } else {
            throw std::invalid_argument("decision.card is neither hard, soft, nor defense");
        }

        if(victim->HP <= 0) {
            newState.setBoardField(decision.objectPos, BoardTile::empty());
            newState.units[1 - this->iActive][ victim->teampos ] = { DEAD_UNIT };
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
    std::tuple<State, uptr<BeginStep>> retVal = { *this, std::make_unique<BeginStep>() };
    State& ret = std::get<0>(retVal);
    ret.iActive = 1 - ret.iActive;
    ret.turnID++;
    ret.timestep = BEGIN;
    //bool dirty = false;
    for(int i = 0; i < ARMY_SIZE; i++) {
        auto& unit = ret.units[ret.iActive][i];
        if( not isDead(unit) ) {
            if(unit->defShieldHP > 0) {
                NullableShared<Character> clone = unit.copy();
                clone->HP += 50;
                clone->defShieldHP = 0;
                std::get<1>(retVal)->resetHP.push_back({ 0, 0, clone->pos });
                ret.units[ret.iActive][i] = std::move(clone);
            }
        }
    }
    ret.checkConsistency();
    return retVal;
}

std::vector<MoveDecision> State::allMovementsForCharacter(const Character& hero) const {
    if(this->timestep == MOVEDfirst && hero.turnMoved == this->turnID)
        return {};
    std::vector<MoveDecision> ret;

    bool boardOfBools[2][FULL_BOARD_WIDTH]; //whether that field has already been passed in the current iteration or not
    for(uint i=0; i<2; i++) for(uint j=0; j<FULL_BOARD_WIDTH; j++) boardOfBools[i][j] = false;

    std::vector<std::tuple<position, bool, std::vector<Direction>>> stack;
    stack.emplace_back(hero.pos, false, std::vector<Direction>());

    while(!stack.empty()) {
        auto [ pos, secondPass, moves ] = stack.back();
        stack.pop_back();

        if(not secondPass) {
            if(not moves.empty()) //do not select the empty list, aka "staying in place"
                ret.emplace_back( hero.pos, pos, moves );
            if(moves.size() < hero.im.mov){
                stack.emplace_back( pos, true, moves );
                boardOfBools[pos.row][pos.column] = true;
                std::vector<std::tuple<Direction, position>> neighbours;
                if(pos.column != 0)
                    neighbours.emplace_back( Direction::LEFT, getNeighbour(pos, Direction::LEFT) );
                if(pos.column != FULL_BOARD_WIDTH - 1)
                    neighbours.emplace_back( Direction::RIGHT, getNeighbour(pos, Direction::RIGHT) );
                neighbours.emplace_back( (pos.row == 0 ? Direction::DOWN : Direction::UP), position(1-pos.row, pos.column) );

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

std::vector<const Character*> State::allAttacksForCharacter(const Character* chr, unsigned int attackingTeam) const {
    std::vector<const Character*> enemies;
    //std::cout << "-|===> for " << chr->uid << chr->team << '@' << chr << '\n';

    assert(chr->team == attackingTeam);
    if(chr->turnMoved == this->turnID and chr->im.rng > 1){
        return {}; //Ranged characters can't attack after they have moved, so we return an empty list
    }
    if(chr->im.usesArcAttack){
        for(int row = 0; row < 2; row++){
            int deltaRow = (chr->pos.row == row ? 0 : 1);
            int min = std::max(chr->pos.column - chr->im.rng + deltaRow, 0);
            int max = std::min(chr->pos.column + chr->im.rng - deltaRow, FULL_BOARD_WIDTH - 1);
            for(int col = min; col <= max; col++){
                const BoardTile& tile = this->board[row][col];
                if(not BoardTile::isEmpty(tile) and tile.team != attackingTeam){
                    enemies.push_back( this->units[1-attackingTeam][tile.index].get() );
                }
            }
        }
    } else {
        auto row = chr->pos.row;
        const BoardTile& tile = this->board[1-row][chr->pos.column];
        if(not BoardTile::isEmpty(tile) and tile.team != attackingTeam){
            enemies.push_back( this->units[1-attackingTeam][tile.index].get() );
        }
        for(int col = chr->pos.column - 1; col >= 0 and col >= chr->pos.column - chr->im.rng; col--){
            const BoardTile& tile = this->board[row][col];
            if(not BoardTile::isEmpty(tile)){
                if(tile.team != attackingTeam){
                    enemies.push_back( this->units[1-attackingTeam][tile.index].get() );
                }
                break;
            }
            if(col != chr->pos.column - chr->im.rng){
                const BoardTile& tile = this->board[1-row][col];
                if(not BoardTile::isEmpty(tile) and tile.team != attackingTeam){
                    enemies.push_back( this->units[1-attackingTeam][tile.index].get() );
                }
            }
        }
        for(int col = chr->pos.column + 1; col < FULL_BOARD_WIDTH and col <= chr->pos.column + chr->im.rng; col++ ){
            const BoardTile& tile = this->board[row][col];
            if(not BoardTile::isEmpty(tile)){
                if(tile.team != attackingTeam){
                    enemies.push_back( this->units[1-attackingTeam][tile.index].get() );
                }
                break;
            }
            if(col != chr->pos.column - chr->im.rng){
                const BoardTile& tile = this->board[1-row][col];
                if(not BoardTile::isEmpty(tile) and tile.team != attackingTeam){
                    enemies.push_back( this->units[1-attackingTeam][tile.index].get() );
                }
            }
        }
    }
    return enemies;
}

std::map<const Character*, std::vector<const Character*>> State::allAttacks() const {
    assert(timestep == Timestep::ABILITYCHOSEN);
    std::map<const Character*, std::vector<const Character*>> ret;
    //print(f' iActive : { this->iActive }, units : { [unit.name for unit in this->units[ this->iActive ] ] }')
    for(uint iChar = 0; iChar < ARMY_SIZE; iChar++){
        const Character* chr = this->units[this->iActive][iChar].get();
        if(isDead(chr)) continue;
        auto enemies = allAttacksForCharacter(chr, this->iActive);

        if(!enemies.empty()){
            auto [ iter, wasInserted ] = ret.insert(std::make_pair<>( chr, enemies ));
            assert(wasInserted);
            (void)iter;
        }
    }
    return ret;
}
