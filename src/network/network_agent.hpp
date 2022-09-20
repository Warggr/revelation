#ifndef REVELATION_NETWORK_AGENT_HPP
#define REVELATION_NETWORK_AGENT_HPP

#include "../agent.hpp"
#include <memory>

class Spectator;
class ServerRoom;

class NetworkAgent : public StepByStepAgent {
    Spectator* sender;
    std::ostream sender_as_ostream;
protected:
    std::ostream& ostream() override{ return sender_as_ostream; }
    uint input(uint min, uint max) override;
public:
    NetworkAgent(uint myId, Spectator* sender);
    NetworkAgent(NetworkAgent&& move) noexcept;
    static std::vector<NetworkAgent> makeAgents(unsigned short int nb, ServerRoom& room, unsigned int startingId = 0);
};

#endif //REVELATION_NETWORK_AGENT_HPP
