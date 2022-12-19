#include "character.hpp"

Character::Character(const ImmutableCharacter& im, char uid):
    uid(uid),
    im(im), HP(im.maxHP), pos(0, 0),
    turnMoved(-1), turnAttacked(-1),
    defShieldHP(0)
{
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
