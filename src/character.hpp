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

    character(uint8_t teampos, const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
              uint8_t rng, float netWorth, const char *flavor, int team);
    uint8_t takeDmg(ActionCard atkType, uint8_t power);  /*{
        HP -= power;
        return power;
    }*/

    short buff(){
        //tempHP += maxHP;
       return HP += 50;
    }

    std::tuple<State, Step> beginTurn();

    nlohmann::json to_json(nlohmann::json &j, const character &character) const;
};

#endif //REVELATION_CHARACTER_HPP
