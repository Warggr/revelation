#include "team.hpp"
#include "step.hpp"
#include "state.hpp"
#include "BoardTile.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BoardTile, team, index)

void to_json(json& j, const Character& chr){
    j = {
        {"name", std::string(chr.name)},
        {"uid", chr.uid},
        {"maxHP", chr.maxHP},
        {"HP", chr.HP},
        {"softAtk", chr.softAtk},
        {"hardAtk", chr.hardAtk},
        {"mov", chr.mov},
        {"rng", chr.rng},
        {"netWorth", chr.netWorth},
        {"flavor", std::string(chr.flavor)}
    };
}

void to_json(json& j, const Player& player){
    (void)j; (void) player; //TODO
}

void to_json(json& j, const State& state) {
    j = {"resourceDeck", state.resDeck.sizeconfig()};
    j["players"] = json(state.players);
    j["board"] = json(state.board);
}

void to_json(json& j, const Team& team){
    j = {
        {"name", team.name},
        {"characters", json(team.characters)}
    };
}

void BeginStep::to_json(json& j) const {
    (void)j; //TODO
}

void MoveStep::to_json(json& j) const {
    (void)j; //TODO
}

void DrawStep::to_json(json& j) const {
    (void)j; //TODO
}

void AbilityStep::to_json(json& j) const {
    (void)j; //TODO
}

void ActionStep::to_json(json& j) const {
    (void)j; //TODO
}