#include "network_agent.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include "semaphore.hpp"
#include <algorithm>

NetworkAgent::NetworkAgent(uint myId, Spectator* sender)
: StepByStepAgent(myId), sender(sender)
{
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
            if (int_val >= 0) {
                uint uint_val = static_cast<uint>(int_val);
                if (min <= uint_val and uint_val <= max) return uint_val;
            }
        } catch(std::invalid_argument&){ }
        sender->send(std::make_shared<std::string>("!Wrong value"));
    }
}

void NetworkAgent::addOption(const std::string_view& option, int) {
    optionList += '"';
    optionList += option;
    optionList += "\",";
}

void NetworkAgent::closeOptionList(const std::string_view& message){
    auto message_summary = std::make_shared<std::string>("[");
    optionList.swap(*message_summary);
    *message_summary += '"';
    *message_summary += message;
    *message_summary += "\"]";
    sender->send(message_summary);
}
