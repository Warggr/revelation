#include "launch_game.hpp"
#include "room.hpp"
#include "network_agent.hpp"
#include "server.hpp"
#include "control/game.hpp"
#include "control/agent.hpp"
#include "search/search.hpp"
#include "search/depthfirstsearch.hpp"
#include "search/loggers.hpp"
#include "nlohmann/json.hpp"
#include <array>
#include <memory>

#define ASSERT(assertion, where, text) if(not (assertion)) throw AgentCreationException(where, text)

std::unique_ptr<SearchAgent> factory(unsigned int myId, const json& j, const json_pointer<json>& ptr){
    std::unique_ptr<Heuristic> heuristic;
    ASSERT(j.contains("heuristic"), ptr, "No search heuristic");
    auto heuristicName = j["heuristic"];
    ASSERT(heuristicName.is_string(), ptr, "Search heuristic is not a string");
    if(heuristicName == "pxt")
        heuristic = std::make_unique<PowerTimesToughnessHeuristic>();
    else
        ASSERT(false, ptr/"heuristic", "Heuristic not recognized");

    std::unique_ptr<SearchPolicy> policy;
    ASSERT(j.contains("policy"), ptr, "No search policy");
    auto policyDescr = j["policy"];
    ASSERT(policyDescr.is_structured(), ptr, "Search policy is not a structured object");
    ASSERT(policyDescr.contains("type"), ptr, "Search policy does not declare a type");
    ASSERT(policyDescr["type"].is_string(), ptr, "Search policy type is not a string");
    if(policyDescr["type"] == "staticDFS")
        policy = std::make_unique<StaticDFS>(std::make_unique<NoOpLogger>(), *heuristic.get());
    else
        ASSERT(false, ptr/"policy"/"type", "Policy not recognized");
    return std::make_unique<SearchAgent>(myId, std::move(policy), std::move(heuristic));
}

AgentDescription parseAgents(const json& j){
    AgentDescription retVal;
    nlohmann::json_pointer<json> ptr;
    ASSERT(j.is_array(), ptr, "Expected an array");
    ASSERT(j.size() == NB_AGENTS, ptr, "Expected 2 elements");
    for(uint i = 0; i<NB_AGENTS; i++){
        auto ptr_agent = ptr/i;
        auto j_agent = j[i];
        ASSERT(j_agent.is_structured(), ptr_agent, "Agent is not a structured object");
        ASSERT(j_agent.contains("type"), ptr_agent/"type", "Agent does not have a type");
        ASSERT(j_agent["type"].is_string(), ptr_agent/"type", "Agent type is not a string");
        auto type = j_agent["type"].get<std::string>();
        if(type == "online") retVal[i].type = AgentDescriptor::NETWORK;
        else if(type == "local") retVal[i].type = AgentDescriptor::LOCAL;
        else if(type == "random") retVal[i].type = AgentDescriptor::RANDOM;
        else if(type == "bot") {
            retVal[i].type = AgentDescriptor::BOT;
            retVal[i].data = reinterpret_cast<void*>(factory(i, j_agent, ptr_agent).release());
        }
        else
            ASSERT(false, ptr_agent/"type", "invalid type");
    }
    return retVal;
}

Agents agentsFromDescription(AgentDescription&& descr, ServerRoom& room){
    Agents retVal;
    for(uint i = 0; i<NB_AGENTS; i++){
        switch(descr[i].type){
            case AgentDescriptor::BOT: retVal[i] = std::unique_ptr<NetworkAgent>(reinterpret_cast<NetworkAgent*>(descr[i].data)); break;
            case AgentDescriptor::LOCAL: retVal[i] = std::make_unique<HumanAgent>(i); break;
            case AgentDescriptor::RANDOM: retVal[i] = std::make_unique<RandomAgent>(i); break;
            case AgentDescriptor::NETWORK: retVal[i] = NetworkAgent::declareUninitializedAgent(room, i); break;
        }
    }
    return retVal;
}

void ServerRoom_impl::launchGame(RoomId id, AgentDescription&& agentsDescr){
    std::cout << "(network thread) Launching game\n";
    Agents agents = agentsFromDescription(std::move(agentsDescr), *this);
    myThread = std::thread([&,id] (Agents&& agents) {
        std::cout << "(game thread) Launching game thread, waiting for agents...\n";
        try {
            for (auto &agent: agents)
                agent->sync_init();
            Game game = Game::createFromAgents(std::move(agents), server->repo);
            std::cout << "(game thread) ...agents found, game in progress...\n";
            game.play(this, false);
            std::cout << "(game thread) ...game finished, ask server for deletion\n";
        }
        catch (TimeoutException &) {}
        catch (DisconnectedException &) {}
        server->askForRoomDeletion(id);
    },
    std::move(agents));
}
