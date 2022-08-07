#include "character.hpp"

using json = nlohmann::json;
char character::s_uid = 'a';

character::character(uint8_t teampos, const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                     uint8_t rng, float netWorth, const char *flavor, int team) : uid(s_uid++), pos(pos) {
    this->teampos = teampos;
    this->name = name;
    this->maxHP = maxHP;
    this->softAtk = softAtk;
    this->hardAtk = hardAtk;
    this->mov = mov;
    this->rng = rng;
    this->netWorth = netWorth;
    this->flavor = flavor;
    this->team = team;
    this->maxAtk = std::max(softAtk, hardAtk);
    this->pos = position(0,0);
    this->turnMoved = NULL;
    this->turnAttacked = NULL;
    this->defShieldHP = 0;
    this->HP = maxHP;
}

json character::to_json(json& j, const character &character) const {
    j = json{{"name", character.name},
             {"cid", character.uid},
             {"maxHP", character.maxHP},
             {"HP", character.HP},
             {"softAtk", character.softAtk},
             {"hardAtk", character.hardAtk},
             {"mov", character.mov},
             {"rng", character.rng},
             {"netWorth", character.netWorth},
             {"flavor", character.flavor}};

    return j;
}

character* character::beginTurn() {
    character* me = new character(this->teampos, this->name, this->maxHP, this->softAtk,this->hardAtk, this->mov,
                                 this->rng, this->netWorth, this->flavor, this->team);

    if(this->defShieldHP > 0) {
        me->HP += 50;
        me->defShieldHP = 0;
    }

    return me;
}

short character::takeDmg(bool isHard, short power) {
    if(this->defShieldHP > 0) {
        short shielded = fmin(this->defShieldHP, power);
        this->defShieldHP -= shielded;
        power -= shielded;
    }
    this->HP -= power;
    return power;
}

short character::getAtk(bool isHard, short turnID) const {
    if(this->turnAttacked == turnID - 1) {
        return 10;
    } else if(isHard) {
        return this->hardAtk;
    } else {
        return this->softAtk;
    }
}

short character::buff() {
    this->defShieldHP = this->maxHP;
    return this->defShieldHP;
}
