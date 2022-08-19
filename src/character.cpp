#include "character.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
char Character::s_uid = 'a';

Character::Character(const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                     uint8_t rng, float netWorth, bool usesArcAttack, const char *flavor) : uid(s_uid++),
                     pos(0, 0), usesArcAttack(usesArcAttack) {
    this->name = name;
    this->maxHP = maxHP;
    this->softAtk = softAtk;
    this->hardAtk = hardAtk;
    this->mov = mov;
    this->rng = rng;
    this->netWorth = netWorth;
    this->flavor = flavor;
    this->maxAtk = std::max(softAtk, hardAtk);
    this->turnMoved = -1;
    this->turnAttacked = -1;
    this->defShieldHP = 0;
    this->HP = maxHP;
}

Character::~Character(){
    for(auto effect_ptr : specialAction)
        delete effect_ptr;
}

short Character::takeDmg(bool isHard, short power) {
    (void) isHard; //currently no difference between hard and soft damage
    if(this->defShieldHP > 0) {
        short shielded = std::min(this->defShieldHP, power);
        this->defShieldHP -= shielded;
        power -= shielded;
    }
    this->HP -= power;
    return power;
}

short Character::getAtk(bool isHard, short turnID) const {
    if(this->turnAttacked == turnID - 1) {
        return 10;
    } else if(isHard) {
        return this->hardAtk;
    } else {
        return this->softAtk;
    }
}

short Character::buff() {
    this->defShieldHP = this->maxHP;
    return this->defShieldHP;
}
