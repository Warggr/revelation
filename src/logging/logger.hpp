#ifndef REVELATION_LOGGER_HPP
#define REVELATION_LOGGER_HPP

#include "gameplay/step.hpp"
#include "setup/team.hpp"
#include "random.hpp"
#include <string_view>
#include <string>
#include <vector>
#include <memory>

class GameRoom;

/**
 * SubLoggers provide one type of logging functionality, e.g. writing to the screen or to a file.
 * A full Logger has zero or more SubLoggers.
 */
class SubLogger {
public:
    virtual ~SubLogger() = default;
    virtual void addStep(std::string_view step) = 0;
};

class Logger {
    std::vector<std::unique_ptr<SubLogger>> subLoggers;
    const std::string startState; //a JSON dump of the start state
public:
    Logger(const State& startState, const std::array<const Team*, 2>& teams, GeneratorSeed seed);
    template<typename SubLoggerT, typename... Args>
    Logger& addSubLogger(Args&&... args){
        subLoggers.push_back(std::make_unique<SubLoggerT>(std::forward<Args>(args)..., startState)); return *this;
    }
    void addStep(const Step& step);
};

#endif //REVELATION_LOGGER_HPP
