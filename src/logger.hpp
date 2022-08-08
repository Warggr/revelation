#ifndef REVELATION_LOGGER_HPP
#define REVELATION_LOGGER_HPP

#include "step.hpp"
#include "team.hpp"

class Logger {
public:
    Logger(std::array<Team, 2>) {};
    virtual ~Logger();
    Logger* liveServer();
    Logger* logToTerminal();
    virtual void addStep(uptr<Step>&&) {};
};

#endif //REVELATION_LOGGER_HPP
