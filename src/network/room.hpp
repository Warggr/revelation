// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef CPPCON2018_SHARED_STATE_HPP
#define CPPCON2018_SHARED_STATE_HPP

#include "spectator.hpp"
#include "listener.hpp"
#include "semaphore.hpp"
#include "net.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <cassert>

// Similar to a Promise<Agent> in Javascript.
// TODO OPTIMIZE have one semaphore for multiple agents
struct WaitingAgent {
    Spectator* agent = nullptr;
    Semaphore promise;
    bool claimed = false;
};

/* Represents a room on the server on which a game takes place (or will take place shortly) */
class ServerRoom {
    std::string greeterMessage; //The message that will be sent to every new spectator
    std::unordered_set<std::shared_ptr<Spectator>> spectators;
    bool firstStep;
    std::unordered_map<AgentId, std::shared_ptr<WaitingAgent>> sessions; //The agents that are supposed to join the room and haven't done so yet
public:
    Server* const server;
    ServerRoom(RoomId id, Server* server);
    ServerRoom(ServerRoom&& move) = default;

    void setGreeterMessage(const std::string& greeterMessage);

    //Create a Spectator and allows it to join once it has done the websocket handshake
    std::shared_ptr<Spectator> addSpectator(tcp::socket& socket, AgentId id = 0);

    void join (Spectator& session);
    void send(const std::string& message);

    std::shared_ptr<WaitingAgent> expectNewAgent(AgentId agentId){
        auto [iter, success] = waitingAgents.insert({ agentId, std::make_shared<WaitingAgent>() });
        assert(success);
        return iter->second;
    }

    void interrupt();

    void reportAfk(Spectator& spec);

    void onConnectAgent(AgentId id, Spectator* agent);

    const std::unordered_set<std::shared_ptr<Spectator>>& getSpectators() const { return spectators; }

    const std::unordered_map<AgentId, std::shared_ptr<WaitingAgent>>& getSessions() const { return sessions; }
};

#ifdef HTTP_CONTROLLED_SERVER
#include <thread>
#include <array>
struct Team;

class ServerRoom_HTTPControlled : public ServerRoom {
    std::thread myThread;
public:
    template<class... Args>
    ServerRoom_HTTPControlled(Args&&... args): ServerRoom(args...) {}
    ServerRoom_HTTPControlled(ServerRoom_HTTPControlled&& move) = default;
    ~ServerRoom_HTTPControlled(){
        if(myThread.joinable()) myThread.join();
    }
    void launchGame(std::array<Team, 2>&& teams, RoomId id);
};
using ServerRoom_impl = ServerRoom_HTTPControlled;
#else
using ServerRoom_impl = ServerRoom;
#endif

#endif
