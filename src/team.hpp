#ifndef REVELATION_TEAM_H
#define REVELATION_TEAM_H
#include "character.hpp"
#include "string"
#include "../cmake-build-debug/_deps/json-src/single_include/nlohmann/json.hpp"

using json = nlohmann::json;
class Team {
private:
    std::string name;
    std::vector<Character> characters;

public:
    Team(std::string name, std::vector<Character> characters);
    json to_json(nlohmann::basic_json<> &j, const Team &team);
};

#endif //REVELATION_TEAM_H
