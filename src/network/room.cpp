// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "room.hpp"
#include "spectator.hpp"
#include "network_agent.hpp"
#include <algorithm>
#include <iostream>

ServerRoom::ServerRoom() = default;

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

void ServerRoom::leave(Spectator& session){
    sessions.erase(&session);
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
    auto iter_agent = waitingAgents.find(id);
    if(iter_agent == waitingAgents.end()) return nullptr; //no such seat
    if(iter_agent->second.claimed) return nullptr; //seat already claimed

    auto ptr = new Spectator(std::move(socket), *this, id);
    return ptr;
}

void ServerRoom::onConnectAgent(AgentId id, Spectator* agent) {
    std::cout << "(async) agent connected\n";
    WaitingAgent waiting = waitingAgents.extract(id).mapped();
    waiting.agent = agent;
    waiting.release_on_connect.release();
}
