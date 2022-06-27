//from serialize import Serializable
#include "position.hpp"
#include "constants.hpp"

struct Character /*: public Serializable */ {
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

    Character(uint8_t teampos, const char* name, short int maxHP, short softAtk, short hardAtk, uint8_t mov, uint8_t rng, float netWorth, const char* flavor):
    teampos(teampos), name(name), maxHP(maxHP), HP(maxHP), softAtk(softAtk), hardAtk(hardAtk), mov(mov), rng(rng), netWorth(netWorth), flavor(flavor),
    uid(s_uid++)
    {};

    uint8_t takeDmg(AtkType atkType, uint8_t power){
        HP -= power;
        return power;
    }
    void buff(){
        //tempHP += maxHP;
        HP += 50;
    }
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
