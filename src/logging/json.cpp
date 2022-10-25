#include "logger.hpp"
#include "gameplay/step_impl.hpp"
#include "gameplay/state.hpp"
#include "setup/team.hpp"
#include "BoardTile.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void to_json(json& j, const BoardTile& tile){
    if(not BoardTile::isEmpty(tile))
        j = { tile.team, tile.index };
}

void to_json(json& j, const Faction& faction){ j = to_string(faction); }

void to_json(json& j, const ActionCard& card){ j = to_string(card); }

void to_json(json& j, const Direction& dir){ j = to_string(dir); }

void to_json(json& j, const ImmutableCharacter& chr){
    j = {
        {"name", std::string(chr.name)},
        {"maxHP", chr.maxHP},
        {"softAtk", chr.softAtk},
        {"hardAtk", chr.hardAtk},
        {"mov", chr.mov},
        {"rng", chr.rng},
        {"netWorth", chr.netWorth},
        {"flavor", std::string(chr.flavor)}
    };
}

void to_json(json& j, const Character& chr){
    to_json(j, chr.im);
    j["cid"] = std::string(1, chr.uid);
    j["HP"] = chr.HP;
}

void to_json(json& j, const Player& player){
    j["actionDeckSize"] = player.deck.sizeconfig();
    j["actions"] = player.getActions();
    j["resources"] = player.getResourceCards();
}

void to_json(json& j, const State& state) {
    j["players"] = state.players;
    j["board"] = state.board;
    auto aliveUnits = json::array();
    for(const auto& team : state.units){
        json j_team;
        for(const auto& unit : team){
            if(unit) j_team.emplace_back(*unit);
            else j_team.emplace_back(nullptr);
        }
        aliveUnits.push_back(j_team);
    }
    j["aliveUnits"] = aliveUnits;
}

json makeStartStateJson(const State& state, const std::array<const Team*, 2>& teams){
    json j = state;
    j["teamNames"] = {  teams[0]->name, teams[1]->name };
    return j;
}
