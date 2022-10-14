#include "step_impl.hpp"
#include "nlohmann/json.hpp"

void to_json(json& j, const position& pos){
    j = {pos.row, pos.column};
}

void make_pass_step(json& j, const char* message){
    j["action"] = "pass";
    j["message"] = message;
}

void BeginStep::to_json(json& j) const {
    j["action"] = "beginTurn";
}

void DrawStep::to_json(json& j) const {
    j["action"] = "draw";
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
    j["target"] = std::string(1, uid);
    j["moves"] = moves;
    j["firstCOF"] = firstCOF;
}

void AbilityStep::to_json(json& j) const {
    if(isPass()) return make_pass_step(j, "Abilities not implemented yet");
    //TODO implement abilities
}

void ActionStep::to_json(json& j) const {
    if(isPass()) return make_pass_step(j, "No action selected");
    j["cardLost"] = cardLost;
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
        j["delete"] = del;
    }
}
