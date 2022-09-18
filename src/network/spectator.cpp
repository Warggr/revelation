// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "spectator.hpp"
#include "connection_list.hpp"
#include <iostream>

Spectator::Spectator(tcp::socket socket, ConnectionList& server) : ws(std::move(socket)), server(server){}

Spectator::~Spectator(){
    // Remove this session from the list of active sessions
    server.leave(*this);
}

void Spectator::fail(error_code ec, char const* what){
    // Don't report these
    if( ec == net::error::operation_aborted || ec == websocket::error::closed) return;
    std::cerr << what << ": " << ec.message() << "\n";
}

void Spectator::on_accept(error_code ec){
    // Handle the error, if any
    if(ec){
        return fail(ec, "accept");
        delete this;
    }

    server.join(*this); // Add this session to the list of active sessions.

    // Read a message
    ws.async_read(
        buffer,
        [&](error_code ec, std::size_t bytes){ on_read(ec, bytes); }
        );
}

void Spectator::on_read(error_code ec, std::size_t){
    // Handle the error, if any
    if(ec) return fail(ec, "read");

    // Send to all connections
    server.send(beast::buffers_to_string(buffer.data()));

    // Clear the buffer
    buffer.consume(buffer.size());

    // Read another message
    ws.async_read(
        buffer,
        [&](error_code ec, std::size_t bytes){
            on_read(ec, bytes);
        });
}

void Spectator::send(const std::shared_ptr<const std::string>& message){
    bool queue_empty = queue_.empty();

    // Always add to queue
    queue_.push_back(message);

    if(queue_empty) {
        // We are not currently writing, so send this immediately
        ws.async_write(
                net::buffer(*queue_.front()),
                [&](error_code ec, std::size_t bytes) {
                    on_write(ec, bytes);
                }
        );
    }
}

void Spectator::on_write(error_code ec, std::size_t){
    // Handle the error, if any
    if(ec) return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws.async_write(
            net::buffer(*queue_.front()),
            [&](error_code ec, std::size_t bytes){
                on_write(ec, bytes);
            });
}
