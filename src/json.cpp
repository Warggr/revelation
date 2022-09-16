#include "team.hpp"
#include "step_impl.hpp"
#include "state.hpp"
#include "BoardTile.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BoardTile, team, index)

void make_pass_step(json& j, const char* message){
    j["action"] = "pass";
    j["message"] = message;
}

void to_json(json& j, const position& pos){
    j["row"] = pos.row;
    j["column"] = pos.column;
}

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

void to_json(json& j, const Player& player){
    j["actionDeckSize"] = player.deck.sizeconfig();
}

void to_json(json& j, const State& state) {
    j["players"] = state.players;
    j["board"] = state.board;
}

void to_json(json& j, const Team& team){
    j["name"] = team.name;
    j["unique_c"] = team.characters_unique;
    j["characters"] = team.characters;
}

void BeginStep::to_json(json& j) const {
    j = {
        {"action", "begin"},
    };
}

void DrawStep::to_json(json& j) const {
    j["type"] = "draw";
    j["newDeckSize"] = json(size);
    if(std::holds_alternative<ActionCard>(cardDrawn)){
        j["clss"] = "action";
        j["value"] = to_string(std::get<ActionCard>(cardDrawn));
    } else {
        j["clss"] = "resource";
        j["value"] = to_string(std::get<Faction>(cardDrawn));
    }
}

void DiscardStep::to_json(json& j) const {
    if(isPass()) return make_pass_step(j, "No card discarded");
    j["action"] = "discard";
}

void MoveStep::to_json(json& j) const {
    if(isPass()) return make_pass_step(j, "Did not move");
    j["action"] = "move";
    j["frm"] = from;
    j["to"] = json(to);
    j["target"] = uid;
    j["moves"] = moves;
    j["firstCOF"] = firstCOF;
}

void AbilityStep::to_json(json& j) const {
    if(isPass()) return make_pass_step(j, "Abilities not implemented yet");
    //TODO implement abilities
}

void ActionStep::to_json(json& j) const {
    if(isPass()) return make_pass_step(j, "No action selected");
    j["cardlost"] = cardLost;
    if(this->cardLost == DEFENSE){
        j["action"] = "def";
        j["subject"] = subject;
        j["temporary"] = def.tempHP;
        j["permanent"] = def.permanentHP;
    } else {
        j["action"] = "atk";
        j["subject"] = subject;
        j["object"] = object;
        j["setLife"] = atk.newHP;
        j["lostLife"] = atk.lostHP;
    }
}
