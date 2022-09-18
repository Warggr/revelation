// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_session.hpp"
#include "spectator.hpp"
#include <iostream>

HttpSession::HttpSession(tcp::socket&& socket, ConnectionList& server)
    : socket_(std::move(socket))
    , server(server)
{
}

void HttpSession::run(){
    // Read a request
    http::async_read(socket_, buffer_, req_,
        [&](error_code ec, std::size_t bytes){
            on_read(ec, bytes);
        });
}

// Report a failure
void HttpSession::fail(error_code ec, char const* what){
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted) return;
    std::cerr << what << ": " << ec.message() << "\n";
}

void HttpSession::on_read(error_code ec, std::size_t){
    // This means they closed the connection
    if(ec == http::error::end_of_stream){
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // Handle the error, if any
    if(ec) return fail(ec, "read");

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(req_)){
        // The websocket connection is established! Transfer the socket and the request to the Server
        server.addSpectator(std::move(socket_)).run(std::move(req_));
        delete this; //don't schedule any further network operations, delete this, and die.
        return;
    }

    // Returns a bad request response
    http::response<http::string_body> res{http::status::bad_request, req_.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req_.keep_alive());
    res.body() = "This server supports only WebSockets.";
    res.prepare_payload();

    // The lifetime of the message has to extend
    // for the duration of the async operation so
    // we use a shared_ptr to manage it.
    using response_type = typename std::decay<decltype(res)>::type;
    auto sp = std::make_shared<response_type>(std::forward<decltype(res)>(res));

    // Write the response
    http::async_write(this->socket_, *sp,
        [&, sp](error_code ec, std::size_t bytes){ on_write(ec, bytes, sp->need_eof()); });
}

void HttpSession::on_write(error_code ec, std::size_t, bool close){
    // Handle the error, if any
    if(ec) return fail(ec, "write");

    if(close){
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        delete this;
        return;
    }

    // Clear contents of the request message,
    // otherwise the read behavior is undefined.
    req_ = {};

    // Read another request
    http::async_read(socket_, buffer_, req_,
        [&](error_code ec, std::size_t bytes){ on_read(ec, bytes); });
}
