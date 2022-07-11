#include "step.hpp"
#include "nlohmann/json.hpp"

Step::Step(std::string type, std::string clss, ActionCard value, int size) {
    this->type = type;
    this->clss = clss;
    this->value = value;
    this->size = size;
}

Step::Step(std::string type, std::string message) {
    this->type = type;
    this->message = message;
}

json Step::to_json(nlohmann::basic_json<> &j, const Step &step) {
    //TODO
}