#include "launch_game.hpp"
#include "room.hpp"
#include "network_agent.hpp"
#include "server.hpp"
#include "control/game.hpp"
#include "control/agent.hpp"
#include "search/search.hpp"
#include "nlohmann/json.hpp"
#include <array>
#include <memory>
#include <fstream>

Agents agentsFromDescription(AgentDescription&& descr, ServerRoom& room){
    Agents retVal;
    for(uint i = 0; i<NB_AGENTS; i++){
        switch(descr[i].type){
            case AgentDescriptor::BOT: retVal[i] = std::unique_ptr<SearchAgent>(reinterpret_cast<SearchAgent*>(descr[i].data)); break;
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
            std::ofstream logFile(path_cat(server->doc_root, std::string("/log_room_") + std::to_string(id) + ".json") );
            std::cout << "(game thread) ...agents found, game in progress...\n";
            game.play(this, &logFile);
            std::cout << "(game thread) ...game finished, ask server for deletion\n";
        }
        catch (TimeoutException &) {}
        catch (DisconnectedException &) {}
        server->askForRoomDeletion(id);
    },
    std::move(agents));
}
