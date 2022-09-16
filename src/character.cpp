#include "character.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
char Character::s_uid = 'a';

Character::Character(const ImmutableCharacter& im):
    uid(s_uid++),
    im(im), HP(im.maxHP), pos(0, 0),
    turnMoved(-1), turnAttacked(-1),
    defShieldHP(0)
{
};

ImmutableCharacter::ImmutableCharacter(const char* name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                     uint8_t rng, float netWorth, bool usesArcAttack, const char* flavor) : usesArcAttack(usesArcAttack) {
    this->name = name;
    this->maxHP = maxHP;
    this->softAtk = softAtk;
    this->hardAtk = hardAtk;
    this->mov = mov;
    this->rng = rng;
    this->netWorth = netWorth;
    this->flavor = flavor;
    this->maxAtk = std::max(softAtk, hardAtk);
}

ImmutableCharacter::~ImmutableCharacter(){
    for(auto effect_ptr : specialAction)
        delete effect_ptr;
}

inline bool rndBool(Generator& gen){ return (gen() >> 6) % 2; } //apparently, the lowest bit is often not that random, so I take the 7th one

ImmutableCharacter ImmutableCharacter::random(Generator& gen){
    return ImmutableCharacter("Bob",
        std::uniform_int_distribution<short>(0, 150)(gen),
        std::uniform_int_distribution<short>(0, 80)(gen),
        std::uniform_int_distribution<short>(0, 80)(gen),
        std::uniform_int_distribution<unsigned char>(0, 5)(gen),
        std::uniform_int_distribution<unsigned char>(0, 5)(gen),
        0,
        rndBool(gen)
    );
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
        return im.hardAtk;
    } else {
        return im.softAtk;
    }
}

void Character::buff() {
    this->defShieldHP = im.maxHP;
}
