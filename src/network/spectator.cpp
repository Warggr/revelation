// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "spectator.hpp"
#include "room.hpp"
#include "control/agent.hpp"
#include <iostream>

constexpr int NB_OUTSIDE_THREADS = 1;

Spectator::Spectator(tcp::socket socket, ServerRoom& room, AgentId id)
: ws(std::move(socket)), room(room), id(id){}

Spectator::~Spectator(){
}

void Spectator::fail(error_code ec, char const* what){
    // Don't report these
    if( ec == net::error::operation_aborted ) return;
    std::cerr << what << ": " << ec.message() << "\n";
}

void Spectator::on_accept(error_code ec){
    std::cout << "(async spectator" << id << ") Accept WS handshake\n";
    connected = true;
    // Handle the error, if any
    if(ec){
        delete this;
        return fail(ec, "accept");
    }

    if(id)
        room.onConnectAgent(id, this);
    room.join(*this); // Add this session to the list of active sessions.

    //Starting the message reading loop
    ws.async_read(
        buffer,
        [&](error_code ec, std::size_t bytes) {
            on_read(ec, bytes);
        }
    );
}

std::string Spectator::get(){
    if(!connected){
        std::cout << "(main) Agent disconnected!\n";
        throw DisconnectedException();
    }
    std::cout << "(main) Getting string, waiting for mutex...\n";
    nb_messages_unread.acquire();
    std::cout << "(main) String mutex acquired\n";
    if(!connected){
        std::cout << "(main) Agent disconnected!\n";
        throw DisconnectedException();
    }
    std::string retVal = reading_queue.front();
    reading_queue.pop();
    return retVal;
}

void Spectator::send(const std::shared_ptr<const std::string>& message){
    std::cout << "(main) Sending message\n";
    bool queue_empty = writing_queue.empty();

    if(not connected) return;
    // Always add to queue
    writing_queue.push(message);

    if(queue_empty) {
        // We are not currently writing, so send this immediately
        ws.async_write(
                net::buffer(*writing_queue.front()),
                [&](error_code ec, std::size_t bytes) {
                    on_write(ec, bytes);
                }
        );
    }
}

void Spectator::on_write(error_code ec, std::size_t){
    std::cout << "(async spectator" << id << ") Message written\n";
    // Handle the error, if any
    if(ec) return fail(ec, "write");

    // Remove the string from the queue
    writing_queue.pop();

    // Send the next message if any
    if(! writing_queue.empty())
        ws.async_write(
            net::buffer(*writing_queue.front()),
            [&](error_code ec, std::size_t bytes){
                on_write(ec, bytes);
            });
}

// This is executed on the network thread, so the only possible race condition is with send() or get()
void Spectator::disconnect(){
    std::cout << "(async spectator" << id << ") disconnecting\n";
    if(!connected) return;
    connected = false;
    nb_messages_unread.unlock();
    //Releasing an additional time in case bad timing causes the main thread to still try to acquire that mutex
    nb_messages_unread.release(NB_OUTSIDE_THREADS);
    room.reportAfk(this);
}

void Spectator::on_read(error_code ec, std::size_t size) {
    if(!connected) return;
    // Handle the error, if any
    if(ec == websocket::error::closed or size == 0){
        std::cout << "(async spectator" << id << ") received close, ask for disconnect\n";
        disconnect();
        return;
    }
    if (ec) return fail(ec, "read");

    std::cout << "(async spectator" << id << ") Read message of size " << size << '\n';

    // Add to queue
    reading_queue.push(beast::buffers_to_string(buffer.data()));
    nb_messages_unread.release();

    // Clear the buffer
    buffer.consume(buffer.size());

    // Read another message
    ws.async_read(
        buffer,
        [&](error_code ec, std::size_t bytes) {
            on_read(ec, bytes);
        }
    );
}
