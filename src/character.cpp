#include "character.hpp"

using json = nlohmann::json;
char character::s_uid = 'a';

character::character(uint8_t teampos, const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                     uint8_t rng, float netWorth, const char *flavor, int team) : uid(s_uid++), pos(0, 0) {
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
    this->turnMoved = -1;
    this->turnAttacked = -1;
    this->defShieldHP = 0;
    this->HP = maxHP;
}

json character::to_json(json& j) const {
    j = json{{"name", name},
             {"cid", uid},
             {"maxHP", maxHP},
             {"HP", HP},
             {"softAtk", softAtk},
             {"hardAtk", hardAtk},
             {"mov", mov},
             {"rng", rng},
             {"netWorth", netWorth},
             {"flavor", flavor}};

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
    (void) isHard; //currently no difference between hard and soft damage
    if(this->defShieldHP > 0) {
        short shielded = std::min(this->defShieldHP, power);
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
