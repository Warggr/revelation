#ifndef REVELATION_TEAM_HPP
#define REVELATION_TEAM_HPP

#include "character.hpp"
#include "nlohmann/json_fwd.hpp"
#include <string>
#include <array>

using json = nlohmann::json;

struct Team {
    std::string name;
    std::array<std::array<Character, ARMY_WIDTH>, 2> characters;

    friend void to_json(json& j, const Team& team);
};

Team mkEurope();
Team mkNearEast();
Team mkFarEast();
Team mkNorthmen();

#endif //REVELATION_TEAM_HPP
