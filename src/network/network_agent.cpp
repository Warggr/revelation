#include "network_agent.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include "semaphore.hpp"
#include <algorithm>

NetworkAgent::NetworkAgent(uint myId, Spectator* sender)
: StepByStepAgent(myId), sender(sender)
{
}

std::vector<std::unique_ptr<NetworkAgent>> NetworkAgent::makeAgents(unsigned short nb, ServerRoom& room, unsigned int startingId){
    using namespace std::chrono_literals;

    std::cout << "(main) make agents\n";
    std::vector<std::shared_ptr<WaitingAgent>> promisedAgents;
    for(int i = 0; i<nb; i++){
        promisedAgents.push_back(room.expectNewAgent(i+startingId+1));
    }
    std::cout << "(main) wait for agents...\n";
    std::vector<std::unique_ptr<NetworkAgent>> retVal;
    for(int i = 0; i<nb; i++) {
        bool success = promisedAgents[i]->promise.try_acquire_for(5min); // "await the promise"
        if(not success) throw TimeoutException();
        if(promisedAgents[i]->agent == nullptr) throw DisconnectedException();
        auto agent = std::make_unique<NetworkAgent>(i+startingId, promisedAgents[i]->agent);
        retVal.push_back(std::move(agent));
    }
    std::cout << "(main) found agents!\n";
    return retVal;
}

uint NetworkAgent::choose(const OptionList& options, const std::string_view& message) {
    std::string optionList = std::string("[");
    for(const auto& [ keys, optionText ] : options){
        optionList += '"';
        optionList += optionText;
        optionList += "\",";
    }
    optionList += '"';
    optionList += message;
    optionList += "\"]";
    sender->send(std::move(optionList));

    input_loop:
        std::string str;
        try {
            str = sender->get_sync();
        } catch(DisconnectedException&){
            throw AgentSurrenderedException(myId);
        }
        auto [ value, success ] = StepByStepAgent::inputValidation( options, str );
        if(not success){
            sender->send(std::string("!Wrong value: `") + str + '`');
            goto input_loop;
        }
    sender->send("Ok");
    return value;
}
