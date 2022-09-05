#include "team.hpp"
#include "effect.hpp"
#include "state.hpp"
#include "random.hpp"
#include <cassert>

class DiscardEffect: public Effect {
    unsigned short int who;
public:
    DiscardEffect(unsigned short int who): who(who) {};
    bool opponentChooses() const override { return true; }
    std::vector<std::string> getOptions(const State& state) const override {
        std::vector<std::string> retVal;
        for(const auto& card : state.players[who].getActions()){
            retVal.push_back(to_string(card));
        }
        return retVal;
    }
    std::unique_ptr<Step> resolve(State& state, unsigned int decision) override {
        state.players[who].discard(decision);
        return nullptr; //TODO create Step subclass for this
    }
};

Team mkNearEast(){
    std::vector<ImmutableCharacter> unique_c; // = {
    unique_c.emplace_back( "Mounted archers", 60, 30, 10, 3, 3, 534.90, true );
    unique_c.emplace_back( "Captives"       , 20, 20, 10, 2, 1, 288.70 );
    unique_c.emplace_back( "Saracens"       , 80, 50, 20, 2, 1, 454.90, false, "Defense 20(light)" );
    unique_c.emplace_back( "Canons"         , 60, 70, 0, 1, 4, 551.60, false );
    unique_c.emplace_back( "Arab officer"   , 100, 10, 10, 2, 1, 355.30 );
    unique_c.emplace_back( "Crossbowman"    , 40, 40, 20, 2, 3, 489.10, false );

    unique_c[1].specialAction.push_front(new DiscardEffect(1));

    return {
        "Near East",
        std::move(unique_c),
        {
            {
                { 1, 2, 3 },
                { 4, 5, 6 }
            }
        }
    };
}

Team mkEurope() {
    std::vector<ImmutableCharacter> unique_c;
    unique_c.emplace_back( "Crossbowman"        , 40, 40, 20, 2, 3, 489.10, false );
    unique_c.emplace_back( "Armored knight"     , 100, 60, 30, 1, 1, 449.90, false, "Defense 20(light)" );
    unique_c.emplace_back( "Fanatics"           , 20, 20, 10, 2, 1, 288.70 );
    unique_c.emplace_back( "Knight"             , 90, 50, 10, 3, 1, 520.50, false, "Defense 20(light)" );
    unique_c.emplace_back( "Lord officer"       , 100, 10, 10, 2, 1, 355.3 );

    return {
        "Europe",
        std::move(unique_c),
        {
            {
                { 1, 2, 3 },
                { 3, 4, 5 }
            }
        }
    };
}

Team Team::random(Generator& generator, unsigned short int nbUnits){
    std::vector<ImmutableCharacter> unique_c;
    std::array<std::array<unsigned short int, ARMY_WIDTH>, 2> characters = {0};

    assert(nbUnits <= ARMY_SIZE);

    for(uint i = 0; i < nbUnits; i++){
        unique_c.push_back(ImmutableCharacter::random(generator));

        //Inefficient algorithm to find an empty space. Doesn't matter too much because this function is executed at most two times
        while(true) {
            auto rnd = generator();
            unsigned short row = rnd % 2, column = (rnd >> 1) % 3;
            if(characters[row][column] != 0) continue;
            else{
                characters[row][column] = i + 1;
                break;
            }
        }
    }
    return { "random team", std::move(unique_c), characters };
}
