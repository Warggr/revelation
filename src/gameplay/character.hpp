#ifndef REVELATION_CHARACTER_HPP
#define REVELATION_CHARACTER_HPP

#include "setup/unique_character.hpp"
#include "position.hpp"
#include "constants.hpp"

struct Character {
    static char s_uid;
    char uid;
    const ImmutableCharacter& im;
    unsigned int team = 3;
    uint8_t teampos = -1;
    short int HP;
    position pos;
    int turnMoved;
    int turnAttacked;
    short defShieldHP;
    bool hasUsedSpecialAction = false;

    Character(const ImmutableCharacter& chr);

    void buff();

    short getAtk(bool isHard, short turnID) const;

    short takeDmg(bool isHard, short power);

    std::forward_list<Effect*> getSpecialAction() { return im.specialAction; }
};

#endif //REVELATION_CHARACTER_HPP
