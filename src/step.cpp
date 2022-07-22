#include "step.hpp"

Step::Step(std::string type, std::string clss, ActionCard value, std::tuple<int, int> size):value(value), from(from), to(to) {
    this->type = type;
    this->clss = clss;
    this->value = value;
    this->size = size;
}

Step::Step(std::string type, position from, position to, char uid, std::vector<Direction> moves, int firstCOF) : from(from), to(to) {
    this->type = type;
    this->from =from;
    this->to = to;
    this->uid = uid;
    this->moves = moves;
    this->firstCOF = firstCOF;
}

Step::Step(std::string type, std::string message): from(from), to(to) {
    this->type = type;
    this->message = message;
}

json Step::to_json(nlohmann::basic_json<> &j, const Step &step) {
    //TODO
}