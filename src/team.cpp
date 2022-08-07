#include "team.hpp"

using json = nlohmann::json;

Team::Team(std::string name,std::vector<std::vector<character>> characters) {
    this->name = name;
    this->characters = characters;
}

json Team::to_json(json& j, const Team &team) {
    j = json {{"name", team.name}};
    j.push_back("characters");
    for(int i = 0; i < team.characters.size(); i++) {
        //j.at("characters").insert(j.at("characters").begin(), team.characters[i].to_json(j, team.characters[i]));
    }
    return j;
}
