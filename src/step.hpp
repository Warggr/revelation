//
// Created by Diana Amirova on 08.07.22.
//

#ifndef REVELATION_STEP_HPP
#define REVELATION_STEP_HPP

#include "string"
#include "constants.hpp"
#include "Deck.hpp"
#include "../cmake-build-debug/_deps/json-src/single_include/nlohmann/json.hpp"
#include "position.hpp"
#include "character.hpp"

using json = nlohmann::json;

class Step {
    std::string type;
    std::string clss;
    ActionCard value;
    int size;
    std::string message;

public:
    Step(std::string type, std::string clss, ActionCard value, int size);
    Step(std::string type, std::string message);
    Step(std::string type, position from, position to, char s_uid, character player);
    json to_json(nlohmann::basic_json<> &j, const Step &step);
};

#endif //REVELATION_STEP_HPP
