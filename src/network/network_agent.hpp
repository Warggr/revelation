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
    static std::vector<std::unique_ptr<NetworkAgent>> makeAgents(unsigned short int nb, ServerRoom& room, unsigned int startingId = 0);
    static inline std::array<std::unique_ptr<Agent>, 2> makeTwoAgents(ServerRoom& room){
        auto vec = makeAgents(2, room);
        return { std::move(vec[0]), std::move(vec[1]) };
    }
};

#endif //REVELATION_NETWORK_AGENT_HPP
