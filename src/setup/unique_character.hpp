#ifndef REVELATION_UNIQUE_CHARACTER_HPP
#define REVELATION_UNIQUE_CHARACTER_HPP

#include "effect.hpp"
#include "random.hpp"
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

#endif //REVELATION_UNIQUE_CHARACTER_HPP
