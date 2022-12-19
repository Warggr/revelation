#ifndef REVELATION_UNITSREPOSITORY_H
#define REVELATION_UNITSREPOSITORY_H

#include "unique_character.hpp"
#include "team.hpp"
#include "visitor.hpp"
#include "constants.hpp"
#include <iosfwd>
#include <unordered_map>
#include <string_view>
#include <array>

using TeamId = std::string;
using CharacterId = std::string; //TODO OPTIMIZE these could be string_views pointing to the character's name
using CharacterRef = const ImmutableCharacter*;

class UnitsRepository {
    using TeamList = std::unordered_map<TeamId, Team>;
    using CharacterList = std::unordered_map<CharacterId, ImmutableCharacter>;
    TeamList teams;
    CharacterList characters;
public:
    const TeamList& getTeams() const { return teams; }
    const CharacterList& getCharacters() const { return characters; }

    template<typename... Args>
    CharacterRef addCharacter(Args&&... args){
        ImmutableCharacter chr(std::forward<Args>(args)...);
        auto [ iter, success ] = characters.emplace(std::make_pair(chr.name, std::move(chr)));
        if(not success) return nullptr;
        else return &iter->second;
    }
    const Team* createTeam(const std::array<CharacterRef, ARMY_SIZE>& cters, const std::string_view& name);
    const Team* createTeam(const std::array<CharacterId, ARMY_SIZE>& names, const std::string_view& name);
    const Team& mkEurope();
    const Team& mkNearEast();
//    const Team& mkFarEast();
//    const Team& mkNorthmen();
    const Team& mkRandom(Generator& generator, unsigned short int nbUnits = ARMY_SIZE);
    void mkDefaultTeams(){
        mkEurope(); mkNearEast();
        //mkFarEast(); mkNorthmen();
    }
};

#endif //REVELATION_UNITSREPOSITORY_H
