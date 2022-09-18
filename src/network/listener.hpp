// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CPPCON2018_LISTENER_HPP
#define CPPCON2018_LISTENER_HPP

#include "net.hpp"
#include <memory>
#include <string>

class ConnectionList;

// Accepts incoming connections and launches the sessions
class Listener {
    tcp::acceptor acceptor;
    tcp::socket socket;
    ConnectionList& server;

    void fail(error_code ec, char const* what);
    void on_accept(error_code ec);
public:
    Listener(net::io_context& ioc, const tcp::endpoint& endpoint, ConnectionList& server);
    void listen(); // Start accepting incoming connections
};

#endif
