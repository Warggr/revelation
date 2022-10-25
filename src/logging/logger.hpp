#ifndef REVELATION_LOGGER_HPP
#define REVELATION_LOGGER_HPP

#include "gameplay/step.hpp"
#include "gameplay/player.hpp"
#include "setup/team.hpp"
#include "nlohmann/json.hpp"
#include <ostream>
#include <vector>
#include <memory>

using json = nlohmann::json;
class ServerRoom;

json makeStartStateJson(const State& startState, const std::array<const Team*, 2>& teams);

/**
 * SubLoggers provide one type of logging functionality, e.g. writing to the screen or to a file.
 * A full Logger has zero or more SubLoggers.
 */
class SubLogger {
public:
    virtual ~SubLogger() = default;
    virtual void addStep(const json& step) = 0;
};

class Logger {
    std::vector<std::unique_ptr<SubLogger>> subLoggers;
    const std::string startState; //a JSON dump of the start state
public:
    Logger(const State& startState, const std::array<const Team*, 2>& teams)
        : startState(makeStartStateJson(startState, teams).dump()) {};
    Logger* liveServer(ServerRoom& server);
    Logger* logToFile(std::ostream& file);
    void addStep(const uptr<Step>& step);
};

#endif //REVELATION_LOGGER_HPP
