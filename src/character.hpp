#ifndef REVELATION_CHARACTER_HPP
#define REVELATION_CHARACTER_HPP

#include "effect.hpp"
#include "position.hpp"
#include "constants.hpp"
#include "random.hpp"
#include "nlohmann/json_fwd.hpp"
#include <forward_list>

struct ImmutableCharacter {
    std::forward_list<Effect*> specialAction;
    const char* name;
    short int maxHP;
    short softAtk;
    short hardAtk;
    short int maxAtk;
    uint8_t mov;
    uint8_t rng;
    float netWorth;
    const char* flavor;
    bool usesArcAttack;

    ImmutableCharacter(const char* name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
        uint8_t rng, float netWorth, bool usesArcAttack = false, const char* flavor = "");
    //ImmutableCharacter manages resources (namely specialAction which needs to be deleted), so it must not have a copy ctor
    ImmutableCharacter(const ImmutableCharacter& copy) = delete;
    ImmutableCharacter(ImmutableCharacter&& move) = default;
    ~ImmutableCharacter();
    static ImmutableCharacter random(Generator& generator);
};

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

    short buff();

    short getAtk(bool isHard, short turnID) const;

    short takeDmg(bool isHard, short power);

    std::forward_list<Effect*> getSpecialAction() { return im.specialAction; }
};

void to_json(nlohmann::json& j, const ImmutableCharacter& chr);

#endif //REVELATION_CHARACTER_HPP
