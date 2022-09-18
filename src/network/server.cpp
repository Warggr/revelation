// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#include "server.hpp"

Server::Server(const char* ipAddress, unsigned short port, const std::string& greeterMessage)
    : connections(greeterMessage)
    , listener( ioc, tcp::endpoint{net::ip::make_address(ipAddress), port}, connections )
{
}
