// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "spectator.hpp"
#include "room.hpp"
#include "control/agent.hpp"
#include <iostream>

constexpr int NB_OUTSIDE_THREADS = 1;

Spectator::Spectator(ServerRoom& room, AgentId id)
: room(room), id(id){}

Spectator& Spectator::claim(tcp::socket &&socket) {
    ws = std::make_unique<websocket::stream<tcp::socket>>(std::move(socket));
    claimed = true;
    return *this;
}

Spectator::~Spectator(){
    assert(not connected);
}

void Spectator::fail(error_code ec, char const* what){
    // Don't report these
    if( ec == net::error::operation_aborted ) return;
    std::cerr << what << ": " << ec.message() << "\n";
}

void Spectator::on_connect(error_code ec){
    std::cout << "(async spectator" << id << ") Accept WS handshake\n";
    if(ec){ // Handle the error, if any
        room.reportAfk(*this);
        return fail(ec, "accept");
    }
    assert(claimed and not connected);
    protectReadingQueue.lock();
    connected = true;
    protectReadingQueue.unlock();
    signalReadingQueue.notify_one();

    room.onConnect(*this);

    //Starting the message reading loop
    ws->async_read(
        buffer,
        [sp=shared_from_this()](error_code ec, std::size_t bytes) {
            sp->on_read(ec, bytes);
        }
    );
}

void Spectator::send(const std::shared_ptr<const std::string>& message){
    std::cout << "(main) Sending message\n";
    bool queue_empty = writing_queue.empty();

    // Always add to queue
    writing_queue.push(message);

    if(not connected) return;
    if(queue_empty) {
        // We are not currently writing, so send this immediately
        ws->async_write(
                net::buffer(*writing_queue.front()),
                [sp=shared_from_this()](error_code ec, std::size_t bytes) {
                    sp->on_write(ec, bytes);
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
        ws->async_write(
            net::buffer(*writing_queue.front()),
            [sp=shared_from_this()](error_code ec, std::size_t bytes){
                sp->on_write(ec, bytes);
            });
}

// This is executed on the network thread, so the only possible race condition is with send() or get()
void Spectator::interrupt(){
    if(!connected) return;
    std::cout << "(async spectator" << id << ") connection interrupted\n";
    protectReadingQueue.lock();
    connected = false; claimed = true;
    protectReadingQueue.unlock();
    signalReadingQueue.notify_one(); //in case anyone was waiting for it
    ws->close(beast::websocket::close_reason());
}

void Spectator::on_read(error_code ec, std::size_t size) {
    if(!connected) return;
    // Handle the error, if any
    if(ec == websocket::error::closed or size == 0){
        std::cout << "(async spectator" << id << ") received close, ask for disconnect\n";
        protectReadingQueue.lock();
        connected = false; claimed = false;
        protectReadingQueue.unlock();
        signalReadingQueue.notify_one();
        room.reportAfk(*this);
        return;
    }
    if (ec) return fail(ec, "read");

    std::cout << "(async spectator" << id << ") Read message of size " << size << '\n';

    // Add to queue
    protectReadingQueue.lock();
    reading_queue.push(beast::buffers_to_string(buffer.data()));
    protectReadingQueue.unlock();
    signalReadingQueue.notify_one();

    // Clear the buffer
    buffer.consume(buffer.size());

    // Read another message
    ws->async_read(
        buffer,
        [sp=shared_from_this()](error_code ec, std::size_t bytes) {
            sp->on_read(ec, bytes);
        }
    );
}

std::string Session::get_sync(){
    while(true) {
        std::cout << "(main) Getting string, waiting for mutex...\n";

        // There are two waiting states: connected (waiting for string...) and disconnected (waiting for connection...)
        // Each can transform into the other. For memory performance reasons, both use the same mutex / cv / semaphore.
        // Both will return quickly if their precondition isn't met.
        // Arbitrarily starting with the connected (wait for string...) state.
        // This is similar to what awaitReconnect() does.
        {
            std::unique_lock<std::mutex> lock(protectReadingQueue);
            if(reading_queue.empty()) {
                if(not connected) continue; // "fail rapidly if the precondition isn't met"
                signalReadingQueue.wait(lock, [&] { return not reading_queue.empty() or not connected; });
            }
            if(not reading_queue.empty()) { //this is not an else - reading_queue will probably have changed since last check
                std::string retVal = std::move(reading_queue.front());
                reading_queue.pop();
                return retVal;
            }
        }

        awaitReconnect();
        std::cout << "(main) String mutex acquired\n";
    }
}

void Session::awaitReconnect() {
    std::unique_lock<std::mutex> lock(protectReadingQueue);
    if(not connected and claimed) throw DisconnectedException(); //this is how the server tells us we're in a "shutting down" state
    if(connected) return; //fail rapidly if the precondition isn't met

    std::cout << "(sync session) Awaiting reconnect...\n";
    using namespace std::chrono_literals;
    if(signalReadingQueue.wait_for(lock, 3min) == std::cv_status::timeout) throw TimeoutException();
    std::cout << "(sync session) ...reconnect signal heard\n";
    if(not connected){
        assert(claimed);
        throw DisconnectedException(); //the server has signaled us but not set us to connected;
    }
    std::cout << "(sync session) reconnected!\n";
}
