#include "network_agent.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include "semaphore.hpp"
#include <algorithm>

NetworkAgent::NetworkAgent(uint myId, Spectator* sender)
: StepByStepAgent(myId), sender(sender), sender_as_ostream(sender->asOStream())
{
}

NetworkAgent::NetworkAgent(NetworkAgent&& move) noexcept
: StepByStepAgent(move), sender(move.sender), sender_as_ostream(sender->asOStream()) {
}

std::vector<NetworkAgent> NetworkAgent::makeAgents(unsigned short int nb, ServerRoom& room, unsigned int startingId){
    std::vector<Spectator*> notConnectedAgents(nb);
    Semaphore semaphore(0);
    for(int i = 0; i<nb; i++){
        room.expectNewAgent(i+1, notConnectedAgents[i], semaphore);
    }
    semaphore.acquire(nb); //waits until all nb threads have released the semaphore once
    std::vector<NetworkAgent> retVal;
    for(int i = 0; i<nb; i++)
        retVal.emplace_back(startingId + i, notConnectedAgents[i]);
    return retVal;
}

uint NetworkAgent::input(uint min, uint max) {
    while(true){
        std::string str = sender->get();
        try {
            long int_val = std::stol(str);
            if (int_val > 0) {
                uint uint_val = static_cast<uint>(int_val);
                if (min <= uint_val and uint_val <= max) return uint_val;
            }
        } catch(std::invalid_argument&){ }
    }
}
