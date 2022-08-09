#ifndef REVELATION_CHARACTER_HPP
#define REVELATION_CHARACTER_HPP

#include "position.hpp"
#include "constants.hpp"
#include "nlohmann/json_fwd.hpp"

struct character {
    int team = -1;
    static char s_uid;
    char uid;
    uint8_t teampos = -1;
    const char* name;
    short int maxHP;
    short softAtk;
    short hardAtk;
    uint8_t mov;
    uint8_t rng;
    float netWorth;
    const char* flavor;
    short int HP;
    position pos;
    short int maxAtk;
    int turnMoved;
    int turnAttacked;
    short defShieldHP;
    bool usesArcAttack;

    character(const char* name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
              uint8_t rng, float netWorth, bool usesArcAttack = false, const char* flavor = "");

    short buff();

    character* beginTurn();

    short getAtk(bool isHard, short turnID) const;

    short takeDmg(bool isHard, short power);
};

void to_json(nlohmann::json& j, const character& chr);

#endif //REVELATION_CHARACTER_HPP
