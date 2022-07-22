//
// Created by Diana Amirova on 08.07.22.
//

#ifndef REVELATION_STEP_HPP
#define REVELATION_STEP_HPP

#include "string"
#include "constants.hpp"
#include "deck.hpp"
#include "../cmake-build-debug/_deps/json-src/single_include/nlohmann/json.hpp"
#include "position.hpp"
#include "character.hpp"

using json = nlohmann::json;

class Step {
    std::string type;
    std::string clss;
    ActionCard value;
    std::tuple<int, int> size;
    std::string message;
    char uid;
    std::vector<Direction> moves;
    int firstCOF;

public:
    position from;
    position to;
    Step(std::string type, std::string clss, ActionCard value, std::tuple<int, int> size);
    Step(std::string type, std::string message);
    Step(std::string type, position from, position to, char uid, std::vector<Direction> moves, int firstCOF);
    json to_json(nlohmann::basic_json<> &j, const Step &step);
};

#endif //REVELATION_STEP_HPP
