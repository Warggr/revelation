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
#include <mutex>

class Session;
class Spectator;
class Server_impl;

/* Represents a room on the server on which a game takes place (or will take place shortly) */
class ServerRoom {
    std::string greeterMessage; //The message that will be sent to every new spectator
    std::unordered_set<std::shared_ptr<Spectator>> spectators;
    bool firstStep;
    std::unordered_map<AgentId, std::shared_ptr<Session>> sessions; //The agents that are supposed to join the room and haven't done so yet
public:
    Server_impl* const server;
    ServerRoom(RoomId id, Server_impl* server);
    ServerRoom(ServerRoom&& move) = default;

    void setGreeterMessage(const std::string& greeterMessage);

    //Create a Spectator and allows it to join once it has done the websocket handshake
    std::shared_ptr<Spectator> addSpectator(tcp::socket& socket, AgentId id = 0);

    void send(const std::string& message);

    std::shared_ptr<Session> addSession(AgentId agentId){
        auto [iter, success] = sessions.insert({ agentId, std::make_shared<Session>(*this, agentId) });
        assert(success);
        return iter->second;
    }

    void interrupt();

    void reportAfk(Spectator& spec);

    void onConnect(Spectator& spectator);

    const std::unordered_set<std::shared_ptr<Spectator>>& getSpectators() const { return spectators; }

    const std::unordered_map<AgentId, std::shared_ptr<Session>>& getSessions() const { return sessions; }
};

#ifdef HTTP_CONTROLLED_SERVER
#include <thread>
#include <array>
struct Team;
#endif

class ServerRoom_impl : public ServerRoom {
#ifdef HTTP_CONTROLLED_SERVER
    std::thread myThread;
#endif
public:
    template<class... Args>
    ServerRoom_impl(Args&&... args): ServerRoom(args...) {}
    ServerRoom_impl(ServerRoom_impl&& move) = default;
#ifdef HTTP_CONTROLLED_SERVER
    ~ServerRoom_impl(){
        if(myThread.joinable()) myThread.join();
    }
    void launchGame(RoomId id);
#endif
};

#endif
