#ifndef REVELATION_NETWORK_AGENT_HPP
#define REVELATION_NETWORK_AGENT_HPP

#include "control/agent.hpp"
#include <vector>
#include <string>

class Spectator;
class ServerRoom;

class NetworkAgent : public StepByStepAgent {
    Spectator* sender;
    std::string optionList = std::string("[");
protected:
    uint input(uint min, uint max) override;
    void addOption(const std::string_view& option, int i) override;
    void closeOptionList(const std::string_view& message) override;
public:
    NetworkAgent(uint myId, Spectator* sender);
    static std::vector<NetworkAgent> makeAgents(unsigned short int nb, ServerRoom& room, unsigned int startingId = 0);
};

#endif //REVELATION_NETWORK_AGENT_HPP
