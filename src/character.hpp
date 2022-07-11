//from serialize import Serializable
#include "position.hpp"
#include "constants.hpp"
#include "nlohmann/json.hpp"
#include "team.hpp"

struct character /*: public Serializable */ {
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

    character(uint8_t teampos, const char *name, short maxHP, short softAtk, short hardAtk, uint8_t mov,
              uint8_t rng, float netWorth, const char *flavor, position pos, int team);
    uint8_t takeDmg(ActionCard atkType, uint8_t power){
        HP -= power;
        return power;
    }
    void buff(){
        //tempHP += maxHP;
        HP += 50;
    }

    void beginTurn() {

    }

    nlohmann::json to_json(nlohmann::json &j, const character &character) const;
};

/*    def serialize(self):
        return {
            "name" : self.name,
            "cid" : self.cid,
            "maxHP" : self.maxHP,
            "HP" : self.HP,
            "tempHP": self.tempHP,
            "softAtk" : self.softAtk,
            "hardAtk" : self.hardAtk,
            "mov" : self.mov,
            "rng" : self.rng,
            "netWorth" : self.netWorth,
            "flavor" : self.flavor
        }
*/
