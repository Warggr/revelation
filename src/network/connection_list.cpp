// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "connection_list.hpp"
#include "spectator.hpp"
#include <algorithm>

ConnectionList::ConnectionList(const std::string& greeterMessage){
    this->greeterMessage = "{\"state\":";
    this->greeterMessage += greeterMessage;
    this->greeterMessage += ",\"steps\":[";
    firstStep = true;
}

void ConnectionList::leave(Spectator& session){
    sessions.erase(&session);
}

void ConnectionList::join(Spectator& session){
    sessions.insert(&session);
    auto message = std::make_shared<const std::string>(greeterMessage + "]}");
    session.send(message);
}

void ConnectionList::send(const std::string& message){
    if(firstStep) firstStep = false;
    else greeterMessage += ',';
    greeterMessage += message;

    // When the callbacks will be executed and the string will actually be sent, the string itself might have gone out of scope.
    // Wrap it in a shared pointer before that happens.
    auto const ss = std::make_shared<const std::string>(message);

    for(const auto& session : sessions)
        session->send(ss);
}

Spectator& ConnectionList::addSpectator(tcp::socket&& socket){
    auto ptr = new Spectator(std::move(socket), *this);
    return *ptr;
}
