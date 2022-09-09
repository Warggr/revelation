#ifndef REVELATION_LOGGER_HPP
#define REVELATION_LOGGER_HPP

#include "step.hpp"
#include "team.hpp"
#include "player.hpp"
#include <ostream>
#include <vector>
#include <memory>

/**
 * SubLoggers provide one type of logging functionality, e.g. writing to the screen or to a file.
 * A full Logger has zero or more SubLoggers.
 */
class SubLogger {
public:
    virtual ~SubLogger() = default;
    virtual void addStep(const Step* step) = 0;
};

class Logger {
    std::vector<std::unique_ptr<SubLogger>> subLoggers;

    const std::array<Player, 2> players;
    const std::array<Team, 2> teams;
public:
    Logger(const std::array<Player, 2>& players, const std::array<Team, 2>& teams): players(players), teams(teams) {};
    Logger* liveServer();
    Logger* logToFile(std::ostream& file);
    void addStep(const uptr<Step>& step);
};

#endif //REVELATION_LOGGER_HPP
