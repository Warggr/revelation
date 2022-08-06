//
// Created by Diana Amirova on 08.07.22.
//

#ifndef REVELATION_STEP_HPP
#define REVELATION_STEP_HPP

#include "string"
#include "constants.hpp"
#include "deck.hpp"
#include "nlohmann/json.hpp"
#include "position.hpp"
#include "character.hpp"
#include <variant>

using json = nlohmann::json;

class Step {
    std::string typ;
public:
    Step() = default;
    Step(std::string typ) {
        this->typ = typ;
    }
    json to_json(nlohmann::basic_json<> &j, const Step &step);
};

class StepOne : public Step{
    std::string msg;

public:
    StepOne(std::string typ, std::string msg) : Step(typ) {
        this->msg = msg;
    }
};

class StepTwo : public Step {
    std::string clss;
    std::variant<ActionCard, Faction> cardDrawn;
    std::tuple<int, int> size;

public:
    StepTwo(std::string typ, std::string clss, std::variant<ActionCard, Faction> cardDrawn, std::tuple<int, int> size) : Step(typ){
        this->clss = clss;
        this->cardDrawn = cardDrawn;
        this->size = size;
    }
};

class StepThree : public Step {
    position from;
    position to;
    char uid;
    std::vector<Direction> moves;
    int firstCOF;

public:
    StepThree(std::string typ, position from, position to, char uid, std::vector<Direction> moves, int firstCOF) : Step(typ), from(from), to(to) {
        this->from = from;
        this->to = to;
        this->moves = moves;
        this->firstCOF = firstCOF;
    }
};

constexpr bool isPass(const Step& step){
    return false; //TODO
}

#endif //REVELATION_STEP_HPP
