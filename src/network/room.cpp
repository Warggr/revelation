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

void ServerRoom_impl::launchGame(RoomId id){
    std::cout << "(network thread) Launching game\n";
    myThread = std::thread([&,id=id]{
        std::cout << "(game thread) Launching game thread, waiting for agents...\n";
        try {
            Game game = Game::createFromAgents(NetworkAgent::makeTwoAgents(*this), server->repo);
            std::cout << "(game thread) ...agents found, game in progress...\n";
            game.play(this, false);
            std::cout << "(game thread) ...game finished, ask server for deletion\n";
        }
        catch(TimeoutException&) {}
        catch(DisconnectedException&) {}
        server->askForRoomDeletion(id);
    });
}
#endif

ServerRoom::ServerRoom(RoomId, Server_impl* server): server(server){}

void ServerRoom::setGreeterMessage(const std::string& newMessage) {
    greeterMessage = "{\"state\":";
    greeterMessage += newMessage;
    greeterMessage += ",\"steps\":[";
    firstStep = true;
    auto message = std::make_shared<std::string>(greeterMessage + "]}");
    for(const auto& client : spectators)
        client->send(message);
    for(const auto& client : sessions)
        client.second->send(message);
}

void ServerRoom::send(const std::string& message){
    // When the callbacks will be executed and the string will actually be sent, the string itself might have gone out of scope.
    // Wrap it in a shared pointer before that happens.
    auto const ss = std::make_shared<const std::string>(message);

    if(firstStep) firstStep = false;
    else greeterMessage += ',';
    greeterMessage += message;

    for(const auto& session : spectators)
        session->send(ss);
    for(const auto& session : sessions)
        session.second->send(ss);
}

std::shared_ptr<Spectator> ServerRoom::addSpectator(tcp::socket& socket, AgentId id){
    if(id != 0){
        auto iter_agent = sessions.find(id);
        if(iter_agent == sessions.end()) return nullptr; //no such seat
        if(iter_agent->second->isClaimed()) return nullptr; //seat already claimed

        iter_agent->second->claim(std::move(socket));
        return iter_agent->second;
    } else {
        auto ptr = std::make_shared<Spectator>(*this);
        ptr->claim(std::move(socket));
        return ptr;
    }
}

void ServerRoom::onConnect(Spectator& spectator) {
    std::cout << "(async) spectator joined\n";
    if(spectator.id == 0){
        spectators.insert(spectator.shared_from_this());
    }

    std::shared_ptr<const std::string> message = std::make_shared<const std::string>(
            this->greeterMessage.empty() ?
            "\"Welcome! The game has not started yet\"" :
            this->greeterMessage + "]}"
    );
    spectator.send(message);
}

void ServerRoom::reportAfk(Spectator& spec){
    if(spec.id == 0)
        spectators.erase(spec.shared_from_this());
}

void ServerRoom::interrupt() { //signals the game that it should end as soon as possible.
    for(auto& [i, session] : sessions){
        session->interrupt();
    }
    for(const auto& spectator: spectators) spectator->interrupt();
}
