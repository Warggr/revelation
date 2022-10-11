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
#include "control/game.hpp"

void ServerRoom_HTTPControlled::launchGame(std::array<Team, 2>&& teams, RoomId id){
    auto array_shared = new std::array<Team, 2>(std::move(teams));
    std::cout << "(network thread) Launching game\n";
    myThread = std::thread([&,array_shared=array_shared,id=id]{
        std::cout << "(game thread) Launching game thread, waiting for agents...\n";
        try {
            Game game(std::move(*array_shared), NetworkAgent::makeTwoAgents(*this));
            std::cout << "(game thread) ...agents found, game in progress...\n";
            game.play(this, false);
            std::cout << "(game thread) ...game finished, ask server for deletion\n";
        }
        catch(TimeoutException&) {}
        catch(DisconnectedException&) {}
        server->askForRoomDeletion(id);
        delete array_shared;
    });
}
#endif

ServerRoom::ServerRoom(RoomId, Server* server): server(server){}

void ServerRoom::setGreeterMessage(const std::string& newMessage) {
    greeterMessage = "{\"state\":";
    greeterMessage += newMessage;
    greeterMessage += ",\"steps\":[";
    firstStep = true;
    auto message = std::make_shared<const std::string>(greeterMessage + "]}");
    for(const auto& client : spectators)
        client->send(message);
}

void ServerRoom::join(Spectator& session){
    std::cout << "(async) spectator joined\n";
    sessions.insert(session.shared_from_this());
    if(not this->greeterMessage.empty()){
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

    for(const auto& session : spectators)
        session->send(ss);
}

std::shared_ptr<Spectator> ServerRoom::addSpectator(tcp::socket& socket, AgentId id){
    if(id != 0){
        auto iter_agent = sessions.find(id);
        if(iter_agent == sessions.end()) return nullptr; //no such seat
        if(iter_agent->second->claimed) return nullptr; //seat already claimed
        iter_agent->second->claimed = true;
    }

    auto ptr = std::make_shared<Spectator>(std::move(socket), *this, id);
    return ptr;
}

void ServerRoom::onConnectAgent(AgentId agentId, Spectator* agent) {
    std::cout << "(async) agent connected\n";
    auto iter = waitingAgents.find(agentId);
    iter->second->agent = agent;
    iter->second->promise.release();
    waitingAgents.erase(iter);
}

void ServerRoom::reportAfk(Spectator& spec){
    if(spec.id == 0)
        spectators.erase(spec.shared_from_this());
}

void ServerRoom::interrupt() { //signals the game that it should end as soon as possible.
    for(auto& [i, waiting] : waitingAgents){
        waiting->agent = nullptr;
        waiting->promise.release();
    }
}
