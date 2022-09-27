// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#include "http_session.hpp"
#include "spectator.hpp"
#include "server.hpp"
#include <iostream>

HttpSession::HttpSession(tcp::socket&& socket, Server& server)
    : socket_(std::move(socket))
    , server(server)
{
    std::cout << "(async listener) Creating HttpSession\n";
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

//Writes the read values to @param roomId and @param agentId and returns a bool to indicate errors, C-style.
bool read_request_path(const boost::string_view& str, RoomId& roomId, AgentId& agentId){
    unsigned int iter = 0;
    if(str[iter++] != '/') return false;
    roomId = 0;
    agentId = 0;
    do{
        char digit = str[iter++] - '0';
        if(0 > digit or digit > 9) return false;
        roomId = 10*roomId + digit;
    } while(str[iter] != '/' and iter < str.size());
    iter++; //consume the /
    do{
        char digit = str[iter++] - '0';
        if(0 > digit or digit > 9) return false;
        agentId = 10*agentId + digit;
    } while(iter < str.size());
    return true;
}

void HttpSession::sendResponse(http::response<http::string_body>&& res){
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);

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

void HttpSession::on_read(error_code ec, std::size_t){
    std::cout << "(async http) read message\n";
    // This means they closed the connection
    if(ec == http::error::end_of_stream){
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        server.askForHttpSessionDeletion(this);
        return;
    }

    // Handle the error, if any
    if(ec){
        server.askForHttpSessionDeletion(this);
        return fail(ec, "read");
    }

    const char* error_message = "Unknown error";

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(req_)){
        std::cout << "(async http) websocket upgrade heard!\n";
        // The websocket connection is established! Transfer the socket and the request to the Server
        RoomId roomId; AgentId agentId;
        if(!read_request_path(req_.target(), roomId, agentId)){
            error_message = "Wrong path";
        } else {
            if (server.rooms.find(roomId) == server.rooms.end()) {
                error_message = "Room not found";
            } else {
                ServerRoom& room = server.rooms.find(roomId)->second;
                //ServerRoom& room = server.rooms[roomId];
                auto spec = room.addSpectator(std::move(socket_), agentId);
                if (!spec) {
                    error_message = "Room did not accept you";
                } else {
                    spec->run(std::move(req_));
                    server.askForHttpSessionDeletion(this); //don't schedule any further network operations, delete this, and die.
                    return;
                }
            }
        }
    } else {
#ifdef HTTP_CONTROLLED_SERVER
        if(req_.target() == "/room" and req_.method() == boost::beast::http::verb::post){
            std::array<Team, 2> teams = { mkEurope(), mkNearEast() };
            auto [roomId, room] = server.addRoom();
            room.launchGame(std::move(teams), roomId);

            http::response<http::string_body> res{ http::status::created, req_.version() };
            res.set(http::field::content_type, "text/plain");
            res.set(http::field::access_control_allow_origin, "*");
            res.keep_alive(req_.keep_alive());
            res.body() = std::to_string(roomId);
            sendResponse(std::move(res));
            return;
        }
#endif
        error_message = "This server only supports WebSockets";
    }

    std::cout << "(async http) prepare response\n";
    std::cout << "(async http) " << error_message << '\n';

    // Returns a bad request response
    http::response<http::string_body> res{http::status::bad_request, req_.version()};
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req_.keep_alive());
    res.body() = error_message;
    sendResponse(std::move(res));
}

void HttpSession::on_write(error_code ec, std::size_t, bool close){
    std::cout << "(async http) write HTTP response\n";
    // Handle the error, if any
    if(ec) return fail(ec, "write");

    if(close){
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        server.askForHttpSessionDeletion(this);
        return;
    }

    // Clear contents of the request message, otherwise the read behavior is undefined.
    req_ = {};

    // Read another request
    http::async_read(socket_, buffer_, req_,
        [&](error_code ec, std::size_t bytes){ on_read(ec, bytes); });
}
