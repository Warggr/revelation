#include <nlohmann/json.hpp>
#include "team.hpp"
#include "character.cpp"

using json = nlohmann::json;

json Team::to_json(json& j, const Team &team) {
    j = json {{"name", team.name}};
    j.push_back("characters");
    for(int i = 0; i < team.characters.size(); i++) {
        j.at("characters").insert(j.at("characters").begin(), team.characters[i].to_json(j, team.characters[i]));
    }
}
