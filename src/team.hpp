#ifndef REVELATION_TEAM_HPP
#define REVELATION_TEAM_HPP

#include "character.hpp"
#include "nlohmann/json_fwd.hpp"
#include <string>
#include <array>

using json = nlohmann::json;

struct Team {
    std::string name;
    std::array<std::array<character, ARMY_WIDTH>, 2> characters;

    friend void to_json(json& j, const Team& team);
};

#endif //REVELATION_TEAM_HPP
