// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CPPCON2018_WEBSOCKET_SESSION_HPP
#define CPPCON2018_WEBSOCKET_SESSION_HPP

#include "semaphore.hpp"
#include "net.hpp"
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <queue>
#include <streambuf>

class ServerRoom;

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;

using AgentId = unsigned short int;

/** Represents an active WebSocket connection to the server. */
class Spectator {
    beast::flat_buffer buffer; //Only used for reading
    websocket::stream<tcp::socket> ws;
    std::queue<std::shared_ptr<const std::string>> writing_queue; //All messages that haven't been sent yet
    std::queue<std::string> reading_queue; //All messages that haven't been read yet
    Semaphore nb_messages_unread;
    ServerRoom& room;
    AgentId id; //Is 0 when we're not an agent, but only a spectator

    void on_accept(error_code ec);
    void on_write(error_code ec, std::size_t bytes_transferred);
    void on_read(error_code ec, std::size_t bytes_transferred);
    void fail(error_code ec, char const* what);
public:
    Spectator(tcp::socket socket, ServerRoom& room, AgentId id = 0);
    ~Spectator();

    //Accepts the websocket handshake asynchronously
    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req){
        ws.async_accept( req, [&](error_code ec){ on_accept(ec); } );
    }

    void send(const std::shared_ptr<const std::string>& message);

    std::string get();
};

#endif
