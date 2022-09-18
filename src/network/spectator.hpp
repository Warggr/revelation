// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CPPCON2018_WEBSOCKET_SESSION_HPP
#define CPPCON2018_WEBSOCKET_SESSION_HPP

#include "net.hpp"
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <vector>

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;

class ConnectionList;

/** Represents an active WebSocket connection to the server
*/
class Spectator {
    beast::flat_buffer buffer;
    websocket::stream<tcp::socket> ws;
    std::vector<std::shared_ptr<const std::string>> queue_;
    ConnectionList& server;

    void fail(error_code ec, char const* what);
    void on_accept(error_code ec);
    void on_read(error_code ec, std::size_t bytes_transferred);
    void on_write(error_code ec, std::size_t bytes_transferred);
public:
    Spectator(tcp::socket socket, ConnectionList& server);
    ~Spectator();

    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req);

    void send(const std::shared_ptr<const std::string>& message);
};

template<class Body, class Allocator>
void Spectator::run(http::request<Body, http::basic_fields<Allocator>> req) {
    // Accept the websocket handshake
    ws.async_accept( req, [&](error_code ec){ on_accept(ec); } );
}

#endif
