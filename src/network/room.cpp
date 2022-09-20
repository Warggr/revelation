// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "room.hpp"
#include "spectator.hpp"
#include "network_agent.hpp"
#include <algorithm>

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
    sessions.insert(&session);
    auto message = std::make_shared<const std::string>(this->greeterMessage + "]}");
    session.send(message);
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

Spectator& ServerRoom::addSpectator(tcp::socket&& socket, AgentId id){
    auto ptr = new Spectator(std::move(socket), *this, id);
    return *ptr;
}

void ServerRoom::onConnectAgent(AgentId id, Spectator* agent) {
    WaitingAgent waiting = waitingAgents.extract(id).mapped();
    waiting.agent = agent;
    waiting.release_on_connect.release();
}
