#include "logger.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void Logger::addStep(const Step& step) {
    std::string message = json(step).dump();
    for(const auto& sub : subLoggers)
        sub->addStep(message);
}
