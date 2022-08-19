#ifndef REVELATION_CHARACTER_HPP
#define REVELATION_CHARACTER_HPP

#include "effect.hpp"
#include "position.hpp"
#include "constants.hpp"
#include "memory.hpp"
#include "nlohmann/json_fwd.hpp"
#include <forward_list>

struct Character {
    unsigned int team = 3;
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
    bool hasUsedSpecialAction = false;
    std::forward_list<Effect*> specialAction;

    Character(const char* name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
              uint8_t rng, float netWorth, bool usesArcAttack = false, const char* flavor = "");
    ~Character();

    short buff();

    short getAtk(bool isHard, short turnID) const;

    short takeDmg(bool isHard, short power);

    std::forward_list<Effect*> getSpecialAction() { return specialAction; }
};

void to_json(nlohmann::json& j, const Character& chr);

#endif //REVELATION_CHARACTER_HPP
