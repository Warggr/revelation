// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef CPPCON2018_ASIO_HPP
#define CPPCON2018_ASIO_HPP

#include <boost/asio.hpp>
#include <exception>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using error_code = boost::system::error_code;

struct DisconnectedException: public std::exception{
};

struct TimeoutException: public std::exception{
};

class ServerRoom; class ServerRoom_HTTPControlled;
class Server; class Server_HTTPFileServer;

#ifdef HTTP_CONTROLLED_SERVER
using Server_impl = Server_HTTPFileServer;
#else
using Server_impl = Server;
#endif

#ifdef HTTP_SERVE_FILES
using ServerRoom_impl = ServerRoom_HTTPControlled;
#else
using ServerRoom_impl = ServerRoom;
#endif

#endif
