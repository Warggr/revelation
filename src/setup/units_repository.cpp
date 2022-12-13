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
    auto MountedArchers = addCharacter( "Mounted archers", 60, 30, 10, 3, 3, 53490, true );
    auto Captives       = addCharacter( "Captives"       , 20, 20, 10, 2, 1, 28870 );
    auto Saracens       = addCharacter( "Saracens"       , 80, 50, 20, 2, 1, 45490, false, "Defense 20(light)" );
    auto Canons         = addCharacter( "Canons"         , 60, 70, 0, 1, 4, 55160, false );
    auto ArabOfficer    = addCharacter( "Arab officer"   , 100, 10, 10, 2, 1, 35530 );

    //Captives->specialAction.push_front(new DiscardEffect(1));

    return *createTeam(
        { MountedArchers, Captives, Saracens, Canons, ArabOfficer, Captives },
        "Near East"
    );
}

const Team& UnitsRepository::mkEurope() {
    auto Crossbowman   = addCharacter( "Crossbowman"        , 40, 40, 20, 2, 3, 48910, false );
    auto ArmoredKnight = addCharacter( "Armored knight"     , 100, 60, 30, 1, 1, 44990, false, "Defense 20(light)" );
    auto Fanatics      = addCharacter( "Fanatics"           , 20, 20, 10, 2, 1, 28870 );
    auto Knight        = addCharacter( "Knight"             , 90, 50, 10, 3, 1, 52050, false, "Defense 20(light)" );
    auto LordOfficer   = addCharacter( "Lord officer"       , 100, 10, 10, 2, 1, 35530 );

    return *createTeam(
        { Crossbowman, ArmoredKnight, Fanatics, Fanatics, Knight, LordOfficer },
        "Europe"
    );
}

void writeRandomCharacters(char* whereTo, unsigned nbCharacters, Generator& generator){
    constexpr int ALPHABET_SIZE = 64;
    constexpr std::string_view alphabet64 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-";
    static_assert(alphabet64.size() == ALPHABET_SIZE);
    for(unsigned i = 0; i<nbCharacters; i++){
        whereTo[i] = alphabet64[ generator() % ALPHABET_SIZE ];
    }
}

const Team& UnitsRepository::mkRandom(Generator& generator, unsigned short int nbUnits){
    std::array<const ImmutableCharacter*, ARMY_SIZE> newCharacters = {nullptr};

    assert(nbUnits <= ARMY_SIZE);

    for(uint fieldsRemaining = ARMY_SIZE; fieldsRemaining > 0; fieldsRemaining--){
        auto rnd = generator();
        bool isThereACharacter = rnd % fieldsRemaining < nbUnits;
        if(isThereACharacter) {
            const ImmutableCharacter* randomCharacter;
            addCharacterToRepo:
                randomCharacter = addCharacter(ImmutableCharacter::random(generator));
                if(not randomCharacter) goto addCharacterToRepo;
            newCharacters[fieldsRemaining] = randomCharacter;
            nbUnits--;
        }
    }
    const Team* retVal = nullptr;

    char newTeamName[] = "random team AAAAA";
    char* randomPart = newTeamName + 12;
    addTeamToRepo:
        writeRandomCharacters(randomPart, 5, generator);
        retVal = createTeam(newCharacters, std::string(newTeamName, sizeof(newTeamName)));
        if(not retVal) goto addTeamToRepo;
    return *retVal;
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

inline bool rndBool(Generator& gen){ return (gen() >> 6) % 2; } //apparently, the lowest bit is often not that random, so I take the 7th one

std::string makeRandomName(Generator& gen){
    std::string_view syllab1[] = {"Sieg", "Ro", "Go", "Ri", "Gi", "C't", "A", "Gan"};
    std::string_view syllab2[] = {"", "mu", "li", "cha", "de", "hu", "chi", "da"};
    std::string_view syllab3[] = {"fried", "lus", "ath", "rd", "on", "l'hu", "lles", "lf"};
    std::string_view titles[] = { "the Brave", "Imperator", "the Invincible", "Lionheart", "Blackblade", "of R'lyeh", "the Hero",  "the Grey" };
    auto rnd = gen();
    std::string retVal;
    retVal += syllab1[ (rnd / 1) % 8 ];
    retVal += syllab2[ (rnd / 8) % 8 ];
    retVal += syllab3[ (rnd / 64) % 8 ];
    retVal += ' ';
    retVal += titles[ (rnd / 512) % 8 ];
    return retVal;
}

ImmutableCharacter ImmutableCharacter::random(Generator& gen){

    return ImmutableCharacter(makeRandomName(gen),
        std::uniform_int_distribution<short>(0, 150)(gen),
        std::uniform_int_distribution<short>(0, 80)(gen),
        std::uniform_int_distribution<short>(0, 80)(gen),
        std::uniform_int_distribution<unsigned char>(0, 5)(gen),
        std::uniform_int_distribution<unsigned char>(0, 5)(gen),
        0,
        rndBool(gen)
    );
}
