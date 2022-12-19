#ifndef REVELATION_UNIQUE_CHARACTER_HPP
#define REVELATION_UNIQUE_CHARACTER_HPP

#include "effect.hpp"
#include "random.hpp"
#include "visitor.hpp"
#include "nlohmann/json_fwd.hpp"
#include <forward_list>
#include <string>
#include <memory>

using json = nlohmann::json;

struct ImmutableCharacter {
#define ImmutableCharacter_ALL(X) \
    X(short, maxHP) \
    X(short, softAtk) \
    X(short, hardAtk) \
    X(uint8_t, mov) \
    X(uint8_t, rng)
#define DECLARE(type, name) type name;
    ImmutableCharacter_ALL(DECLARE)
#undef DECLARE
    std::string name;
    const std::string slug;
    std::forward_list<Effect*> specialAction;
    short maxAtk;
    unsigned int netWorth = 0;
    std::string flavor = {};
    bool usesArcAttack = false;

#define WRITE(type, name) type name,
    ImmutableCharacter(std::string name, std::string slug, ImmutableCharacter_ALL(WRITE)
                       unsigned int netWorth, bool usesArcAttack = false, const char* flavor = "");
#undef WRITE
    ImmutableCharacter(WriterVisitor& visitor);
    //ImmutableCharacter manages resources (namely specialAction which needs to be deleted), so it must not have a copy ctor
    ImmutableCharacter(const ImmutableCharacter& copy) = delete;
    ImmutableCharacter(ImmutableCharacter&& move) = default;
    ~ImmutableCharacter();
    static ImmutableCharacter random(Generator& generator);
    friend void to_json(json& j, const ImmutableCharacter& character);
};

#endif //REVELATION_UNIQUE_CHARACTER_HPP
