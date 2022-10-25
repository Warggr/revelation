#include "team.hpp"
#include "units_repository.hpp"
#include "effect.hpp"
#include "gameplay/state.hpp"
#include "random.hpp"
#include <cassert>

const Team* UnitsRepository::createTeam(const std::array<CharacterRef, ARMY_SIZE>& cters, const std::string_view& name){
    Team team = { std::string(name), {} };
    for(unsigned i = 0; i<2; i++){
        for(unsigned j = 0; j<ARMY_WIDTH; j++){
            team.characters[i][j] = cters[i*ARMY_WIDTH + j];
        }
    }
    auto [ iter, success ] = teams.insert( std::make_pair( name, std::move(team) ) );
    if(not success) return nullptr;
    else return &iter->second;
}
const Team* UnitsRepository::createTeam(const std::array<CharacterId, ARMY_SIZE>& names, const std::string_view& name){
    std::array<const ImmutableCharacter*, 6> usedCharacters = { nullptr };
    for(unsigned i = 0; i<ARMY_SIZE; i++){
        if(names[i] == "") usedCharacters[i] = nullptr;
        else{
            auto iter = characters.find(names[i]);
            if(iter == characters.end()) return nullptr;
            usedCharacters[i] = &iter->second;
        }
    }
    return createTeam(usedCharacters, name);
}

/*
class DiscardEffect: public Effect {
    unsigned short int who;
public:
    DiscardEffect(unsigned short int who): who(who) {};
    bool opponentChooses() const override { return true; }
    std::vector<std::string> getOptions(const State& state) const override {
        std::vector<std::string> retVal;
        for(const auto& card : state.players[who].getActions()){
            retVal.push_back(std::string(to_string(card)));
        }
        return retVal;
    }
    std::unique_ptr<Step> resolve(State& state, unsigned int decision) override {
        state.players[who].discardAction(decision);
        return nullptr; //TODO create Step subclass for this
    }
};
*/

const Team& UnitsRepository::mkNearEast(){
    auto MountedArchers = addCharacter( "Mounted archers", 60, 30, 10, 3, 3, 534.90, true );
    auto Captives       = addCharacter( "Captives"       , 20, 20, 10, 2, 1, 288.70 );
    auto Saracens       = addCharacter( "Saracens"       , 80, 50, 20, 2, 1, 454.90, false, "Defense 20(light)" );
    auto Canons         = addCharacter( "Canons"         , 60, 70, 0, 1, 4, 551.60, false );
    auto ArabOfficer    = addCharacter( "Arab officer"   , 100, 10, 10, 2, 1, 355.30 );

    //Captives->specialAction.push_front(new DiscardEffect(1));

    return *createTeam(
        { MountedArchers, Captives, Saracens, Canons, ArabOfficer, Captives },
        "Near East"
    );
}

const Team& UnitsRepository::mkEurope() {
    auto Crossbowman   = addCharacter( "Crossbowman"        , 40, 40, 20, 2, 3, 489.10, false );
    auto ArmoredKnight = addCharacter( "Armored knight"     , 100, 60, 30, 1, 1, 449.90, false, "Defense 20(light)" );
    auto Fanatics      = addCharacter( "Fanatics"           , 20, 20, 10, 2, 1, 288.70 );
    auto Knight        = addCharacter( "Knight"             , 90, 50, 10, 3, 1, 520.50, false, "Defense 20(light)" );
    auto LordOfficer   = addCharacter( "Lord officer"       , 100, 10, 10, 2, 1, 355.3 );

    return *createTeam(
        { Crossbowman, ArmoredKnight, Fanatics, Fanatics, Knight, LordOfficer },
        "Europe"
    );
}

const Team& UnitsRepository::mkRandom(Generator& generator, unsigned short int nbUnits){
    std::array<const ImmutableCharacter*, ARMY_SIZE> newCharacters = {nullptr};

    assert(nbUnits <= ARMY_SIZE);

    for(uint i = 0; i < nbUnits; i++){
        auto randomCharacter = addCharacter(ImmutableCharacter::random(generator));

        //Inefficient algorithm to find an empty space. Doesn't matter too much because this function is executed at most two times
        while(true) {
            auto rnd = generator();
            unsigned short pos = rnd % 6;
            if(newCharacters[pos] != nullptr) continue;
            else{
                newCharacters[pos] = randomCharacter;
                break;
            }
        }
    }
    return *createTeam( newCharacters, "random team" ); // TODO give random name, or allow name collisions in the repository
}

ImmutableCharacter::ImmutableCharacter(WriterVisitor& visitor)
: name(visitor.get("name", (std::string*)nullptr))
{
#define VISIT(type, x) x = visitor.get(#x, (type*)nullptr);
    ImmutableCharacter_ALL(VISIT)
#undef VISIT_TYPED
    bool defaultUAA = false;
    usesArcAttack = visitor.get("usesArcAttack", &defaultUAA);
    float defaultNetWorth = 0;
    netWorth = visitor.get("netWorth", &defaultNetWorth);
    std::string defaultFlavor = {};
    flavor = visitor.get("flavor", &defaultFlavor);
    maxAtk = hardAtk > softAtk ? hardAtk : softAtk;
}
