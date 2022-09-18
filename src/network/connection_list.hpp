// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef CPPCON2018_SHARED_STATE_HPP
#define CPPCON2018_SHARED_STATE_HPP

#include "spectator.hpp"
#include "listener.hpp"
#include "net.hpp"
#include <string>
#include <unordered_set>

class ConnectionList {
    std::string greeterMessage;
    std::unordered_set<Spectator*> sessions; //All these pointers are owning (they could've been unique_ptr's)
    bool firstStep;
public:
    ConnectionList(const std::string& greeterMessage);
    ~ConnectionList(){ for(const auto& session: sessions) delete session; }

    //Create a Spectator and allows it to join once it has done the websocket handshake
    Spectator& addSpectator(tcp::socket&& socket);

    void join (Spectator& session);
    void leave (Spectator& session);
    void send  (const std::string& message);
};

#endif
