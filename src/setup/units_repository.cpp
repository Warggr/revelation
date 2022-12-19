#include "team.hpp"
#include "units_repository.hpp"
#include "effect.hpp"
#include "gameplay/state.hpp"
#include "www_visitor.hpp"
#include "random.hpp"
#include <cassert>
#include <fstream>
#include <exception>

using TeamList = UnitsRepository::TeamList;

std::pair<TeamList::iterator, bool> UnitsRepository::p_createTeam(const std::array<CharacterRef, ARMY_SIZE>& cters, const std::string_view& name){
    Team team = { std::string(name), {} };
    for(unsigned i = 0; i<2; i++){
        for(unsigned j = 0; j<ARMY_WIDTH; j++){
            team.characters[i][j] = cters[i*ARMY_WIDTH + j];
        }
    }
    return teams.insert( std::make_pair( name, std::move(team) ) );
}
std::pair<TeamList::iterator, bool> UnitsRepository::p_createTeam(const std::array<CharacterId, ARMY_SIZE>& names, const std::string_view& name){
    std::array<const ImmutableCharacter*, 6> usedCharacters = { nullptr };
    for(unsigned i = 0; i<ARMY_SIZE; i++){
        if(names[i] == "") usedCharacters[i] = nullptr;
        else{
            auto iter = characters.find(names[i]);
            if(iter == characters.end()) return std::make_pair(teams.end(), false);
            usedCharacters[i] = &iter->second;
        }
    }
    return p_createTeam(usedCharacters, name);
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

std::pair<TeamList::iterator, bool> UnitsRepository::p_createTeam(WriterVisitor& visitor) {
    std::array<CharacterId, NB_CHARACTERS> characterNames;
    char buffer[] = "0";
    std::string noCharacter;
    for(unsigned char i = 0; i<characterNames.size(); i++){
        buffer[0] = '0' + i;
        std::string_view sv(buffer, 1);
        characterNames[i] = visitor.get(sv, &noCharacter);
    }
    std::string teamName = visitor.get("name", (std::string*)nullptr);
    if(not visitor.empty())
        throw std::invalid_argument(std::string("Extra value: ").append(visitor.anyKey()));
    return p_createTeam(characterNames, teamName);
}

void UnitsRepository::readFile(std::istream& file){
    std::string line;
    enum { CHARACTERS, TEAMS } readingMode = CHARACTERS;
    while(getline(file, line)){
        if(line.empty()) continue;
        if(line[0] == '['){
            if(line == "[Characters]") readingMode = CHARACTERS;
            else if(line == "[Teams]") readingMode = TEAMS;
            else assert(false);
        } else {
            WwwDataVisitor visitor(line);
            if(readingMode == CHARACTERS){
                addCharacter( visitor );
            }
            else if(readingMode == TEAMS){
                createTeam( visitor );
            }
            else assert(false);
        }
    }
}

void UnitsRepository::mkDefaultTeams(){
    std::ifstream infile("teams.txt");
    if(not infile.is_open())
        throw std::runtime_error("Missing teams.txt");
    readFile(infile);
}

void writeRandomCharacters(char* whereTo, unsigned nbCharacters, Generator& generator){
    constexpr int ALPHABET_SIZE = 64;
    constexpr std::string_view alphabet64 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+-";
    static_assert(alphabet64.size() == ALPHABET_SIZE);
    for(unsigned i = 0; i<nbCharacters; i++){
        whereTo[i] = alphabet64[ generator() % ALPHABET_SIZE ];
    }
}

TeamList::iterator UnitsRepository::addTeamWithoutName(const std::array<CharacterRef, ARMY_SIZE>& cters, Generator& gen) {
    auto retVal = teams.end();
    char newTeamName[] = "random team AAAAA";
    char* randomPart = newTeamName + 12;
    addTeamToRepo:
        bool success;
        writeRandomCharacters(randomPart, 5, gen);
        std::tie(retVal, success) = p_createTeam(cters, std::string(newTeamName, sizeof(newTeamName)));
        if(not success) goto addTeamToRepo;
    return retVal;
}

TeamList::iterator UnitsRepository::mkRandomWithPreexistingCharacters(Generator& generator){
    std::array<const ImmutableCharacter*, ARMY_SIZE> newCharacters = {nullptr};

    for(uint i = 0; i<ARMY_SIZE; i++){
        auto rnd = generator() % characters.size();
        auto iter = characters.begin(); std::advance(iter, rnd);
        newCharacters[i] = &iter->second;
    }
    return addTeamWithoutName(newCharacters, generator);
}

TeamList::iterator UnitsRepository::mkRandom(Generator& generator, unsigned short int nbUnits){
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
    return addTeamWithoutName(newCharacters, generator);
}

ImmutableCharacter::ImmutableCharacter(std::string name, std::string slug, short maxHP, short softAtk, short hardAtk, uint8_t mov,
                                       uint8_t rng, unsigned netWorth, bool usesArcAttack, const char* flavor)
: name(std::move(name)), slug(std::move(slug)), usesArcAttack(usesArcAttack) {
    this->maxHP = maxHP;
    this->softAtk = softAtk;
    this->hardAtk = hardAtk;
    this->mov = mov;
    this->rng = rng;
    this->netWorth = netWorth;
    this->flavor = flavor;
    this->maxAtk = std::max(softAtk, hardAtk);
}

ImmutableCharacter::~ImmutableCharacter(){
    for(auto effect_ptr : specialAction)
        delete effect_ptr;
}

ImmutableCharacter::ImmutableCharacter(WriterVisitor& visitor)
: name(visitor.get("name", (std::string*)nullptr))
, slug(visitor.get("slug", (std::string*)nullptr))
{
#define VISIT(type, x) x = visitor.get(#x, (type*)nullptr);
    ImmutableCharacter_ALL(VISIT)
#undef VISIT
    bool defaultUAA = false;
    usesArcAttack = visitor.get("usesArcAttack", &defaultUAA);
    unsigned int defaultNetWorth = 0;
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

short int mkRandomNumberWithOutliers(short min, short max, Generator& gen){
    short retVal = std::uniform_int_distribution<short>(min, max + 3)(gen);
    if(retVal > max){
        retVal = (retVal - max) * max;
    }
    return retVal;
}

ImmutableCharacter ImmutableCharacter::random(Generator& gen){
    char slug[6]; writeRandomCharacters(slug, 6, gen);
    return ImmutableCharacter(makeRandomName(gen), slug,
        10 * mkRandomNumberWithOutliers(0, 15, gen),
        10 * mkRandomNumberWithOutliers(0, 8, gen),
        10 * mkRandomNumberWithOutliers(0, 8, gen),
        std::uniform_int_distribution<unsigned char>(0, 5)(gen),
        std::uniform_int_distribution<unsigned char>(0, 5)(gen),
        0,
        rndBool(gen)
    );
}
