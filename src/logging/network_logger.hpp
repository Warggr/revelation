#ifndef REVELATION_NETWORK_LOGGER_HPP
#define REVELATION_NETWORK_LOGGER_HPP

#include "logger.hpp"
#include "network/room.hpp"
#include <memory>

class LiveServerAndLogger : public SubLogger {
    GameRoom& serverRoom;
    std::shared_ptr<bool> cancellation_token = std::make_shared<bool>(false);
public:
    LiveServerAndLogger(GameRoom& serverRoom, std::string_view start);
    ~LiveServerAndLogger() override {
        *cancellation_token = true;
    }
    void addStep(std::string_view step) override;
};

#endif //REVELATION_NETWORK_LOGGER_HPP
