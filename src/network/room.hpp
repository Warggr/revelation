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
#ifdef HTTP_CONTROLLED_SERVER
#include "control/game.hpp"
#include <thread>
#include <array>
struct Team;
#endif

using RoomId = unsigned short int;

struct WaitingAgent {
    Spectator*& agent;
    Semaphore* release_on_connect;
    bool claimed;
};

/* Represents a room on the server on which a game takes place (or will take place shortly) */
class ServerRoom {
    std::string greeterMessage; //The message that will be sent to every new spectator
    std::unordered_set<Spectator*> sessions; //All these pointers are owning (they could've been unique_ptr's)
    bool firstStep;
    std::unordered_map<AgentId, WaitingAgent> waitingAgents; //The agents that are supposed to join the room and haven't done so yet
    Server* const server;
#ifdef HTTP_CONTROLLED_SERVER
    std::thread myThread;
#endif
public:
    ServerRoom(RoomId id, Server* server);
    ~ServerRoom(){
#ifdef HTTP_CONTROLLED_SERVER
        if(myThread.joinable()) myThread.join();
#endif
        for(const auto& session: sessions) delete session;
    }
    ServerRoom(ServerRoom&& move) = default;
#ifdef HTTP_CONTROLLED_SERVER
    void launchGame(std::array<Team, 2>&& teams, RoomId id);
#endif
    void setGreeterMessage(const std::string& greeterMessage);

    //Create a Spectator and allows it to join once it has done the websocket handshake
    Spectator* addSpectator(tcp::socket&& socket, AgentId id = 0);

    void join (Spectator& session);
    void send  (const std::string& message);

    void expectNewAgent(AgentId agentId, Spectator*& agent, Semaphore* release_once_found){
        waitingAgents.insert({ agentId, {agent, release_once_found, false} });
    }

    void interrupt(){ //signals the game that it should end as soon as possible.
        for(const auto session: sessions) session->disconnect();
    }

    void reportAfk(Spectator* spec);

    void onConnectAgent(AgentId id, Spectator* agent);
};

#endif