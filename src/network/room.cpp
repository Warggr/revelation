// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "room.hpp"
#include "spectator.hpp"
#include "network_agent.hpp"
#include "server.hpp"
#include <algorithm>
#include <iostream>

#ifdef HTTP_CONTROLLED_SERVER
void ServerRoom::launchGame(std::array<Team, 2>&& teams, RoomId id){
    auto array_shared = new std::array<Team, 2>(std::move(teams));
    std::cout << "(network thread) Launching game\n";
    myThread = std::thread([&,array_shared=array_shared,id=id]{
        std::cout << "(game thread) Launching game thread, waiting for agents...\n";
        Game game(std::move(*array_shared), NetworkAgent::makeTwoAgents(*this));
        std::cout << "(game thread) ...agents found, game in progress...\n";
        game.play(this, false);
        std::cout << "(game thread) ...game finished, ask server for deletion\n";
        server->askForRoomDeletion(id);
        delete array_shared;
    });
}
#endif

ServerRoom::ServerRoom(RoomId, Server* server): server(server){};

void ServerRoom::setGreeterMessage(const std::string& greeterMessage) {
    this->greeterMessage = "{\"state\":";
    this->greeterMessage += greeterMessage;
    this->greeterMessage += ",\"steps\":[";
    firstStep = true;
    for(const auto& client : sessions){
        auto message = std::make_shared<const std::string>(this->greeterMessage + "]}");
        client->send(message);
    }
}

void ServerRoom::join(Spectator& session){
    std::cout << "(async) spectator joined\n";
    sessions.insert(&session);
    if(this->greeterMessage.size() != 0){
        auto message = std::make_shared<const std::string>(this->greeterMessage + "]}");
        session.send(message);
    } else {
        auto message = std::make_shared<const std::string>("Welcome! The game has not started yet");
        session.send(message);
    }
}

void ServerRoom::send(const std::string& message){
    if(firstStep) firstStep = false;
    else greeterMessage += ',';
    greeterMessage += message;

    // When the callbacks will be executed and the string will actually be sent, the string itself might have gone out of scope.
    // Wrap it in a shared pointer before that happens.
    auto const ss = std::make_shared<const std::string>(message);

    for(const auto& session : sessions)
        session->send(ss);
}

Spectator* ServerRoom::addSpectator(tcp::socket&& socket, AgentId id){
    if(id != 0){
        auto iter_agent = waitingAgents.find(id);
        if(iter_agent == waitingAgents.end()) return nullptr; //no such seat
        if(iter_agent->second.claimed) return nullptr; //seat already claimed
        iter_agent->second.claimed = true;
    }

    auto ptr = new Spectator(std::move(socket), *this, id);
    return ptr;
}

void ServerRoom::onConnectAgent(AgentId agentId, Spectator* agent) {
    std::cout << "(async) agent connected\n";
    WaitingAgent waiting = waitingAgents.extract(agentId).mapped();
    waiting.agent = agent;
    if(waiting.release_on_connect) waiting.release_on_connect->release();
}

void ServerRoom::reportAfk(Spectator* spec){
    if(spec->id == 0){ //delete only if it is a spectator. Agents can't be deleted as long as they are used by the game
        sessions.erase(spec);
        delete spec;
    }
}