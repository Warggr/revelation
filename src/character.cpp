#include "character.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
char character::s_uid = 'a';

character::character(uint8_t teampos, const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                     uint8_t rng, float netWorth, const char *flavor, position pos, int team) : uid(s_uid++), pos(pos) {
    this->teampos = teampos;
    this->name = name;
    this->maxHP = maxHP;
    this->softAtk = softAtk;
    this->hardAtk = hardAtk;
    this->mov = mov;
    this->rng = rng;
    this->netWorth = netWorth;
    this->flavor = flavor;
    this->pos = pos;
    this->team = team;
}

json character::to_json(json& j, const character &character) const {
    j = json{{"name", character.name},
             {"cid", character.uid},
             {"maxHP", character.maxHP},
             {"HP", character.HP},
             {"softAtk", character.softAtk},
             {"hardAtk", character.hardAtk},
             {"mov", character.mov},
             {"rng", character.rng},
             {"netWorth", character.netWorth},
             {"flavor", character.flavor}};

    return j;
}



