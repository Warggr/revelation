#ifndef REVELATION_TEAM_HPP
#define REVELATION_TEAM_HPP

#include "string"
#include "character.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
class Team {
private:
    std::string name;

public:
    std::vector<std::vector<character>> characters;
    Team(std::string name, std::vector<std::vector<character>> characters);
    json to_json(nlohmann::basic_json<> &j, const Team &team);
};

#endif //REVELATION_TEAM_HPP
