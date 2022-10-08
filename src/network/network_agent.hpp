#ifndef REVELATION_NETWORK_AGENT_HPP
#define REVELATION_NETWORK_AGENT_HPP

#include "control/agent.hpp"
#include <vector>
#include <string>

class Spectator;
class ServerRoom;

class NetworkAgent : public StepByStepAgent {
    Spectator* sender;
protected:
    uint input(uint min, uint max) override { (void) max; return min; /* TODO FEATURE */ }
    uint choose(const OptionList& list, const std::string_view& message) override;
public:
    NetworkAgent(uint myId, Spectator* sender);
    static std::vector<std::unique_ptr<NetworkAgent>> makeAgents(unsigned short int nb, ServerRoom& room, unsigned int startingId = 0);
    static inline std::array<std::unique_ptr<Agent>, 2> makeTwoAgents(ServerRoom& room){
        auto vec = makeAgents(2, room);
        return { std::move(vec[0]), std::move(vec[1]) };
    }
};

#endif //REVELATION_NETWORK_AGENT_HPP
