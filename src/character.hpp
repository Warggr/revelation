//from serialize import Serializable

#ifndef REVELATION_CHARACTER_HPP
#define REVELATION_CHARACTER_HPP

#include "position.hpp"
#include "constants.hpp"
#include "../cmake-build-debug/_deps/json-src/single_include/nlohmann/json.hpp"

struct character {
public:
    int team;
    static char s_uid;
    char uid;
    uint8_t teampos;
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

    character(uint8_t teampos, const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
              uint8_t rng, float netWorth, const char *flavor, int team);
    uint8_t takeDmg(ActionCard atkType, uint8_t power);

    short buff();

    character* beginTurn();

    nlohmann::json to_json(nlohmann::json &j, const character &character) const;

    short getAtk(bool isHard, short turnID);

    short takeDmg(bool isHard, short power);
};

#endif //REVELATION_CHARACTER_HPP
