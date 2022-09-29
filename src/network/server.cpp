// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#include "server.hpp"
#include "http_session.hpp"
#ifdef HTTP_CONTROLLED_SERVER
#include "setup/team.hpp"
#endif
#include <thread>

Server::Server(const char* ipAddress, unsigned short port)
    : listener( *this, ioc, tcp::endpoint{net::ip::make_address(ipAddress), port} )
{
    std::cout << "(main) Starting server\n";
}

Server::~Server(){
    for(const HttpSession* session : sessions){
        delete session;
    }
}

void Server::start(){
    std::cout << "(network) Starting server...\n";
    //! this function runs indefinitely.
    //! To stop it, you need to call stop() (presumably from another thread)
    listener.listen();

#ifdef HTTP_CONTROLLED_SERVER
    addRoom(0).second.launchGame({ mkEurope(), mkNearEast() }, 0);
#endif
    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code const&, int signal){
        std::cout << "Received signal " << signal << ". Stopping...\n";
        // Stop the io_context. This will cause run()
        // to return immediately, eventually destroying the
        // io_context and any remaining handlers in it.
        stop();
    });

    ioc.run();
}

void Server::stop(){
    std::cout << "(network) ...stopping server and interrupting rooms\n";
    for(auto& [id, room] : rooms){
        room.interrupt();
    }
    ioc.stop(); //Stop io_context. Now the only things that can block are in the game threads.
    rooms.clear(); //Closing all rooms (some might wait for their game to end)
}

std::pair<RoomId, ServerRoom&> Server::addRoom(RoomId newRoomId) {
    std::cout << "(main) Add room to server\n";
    auto [iter, success] = rooms.insert({newRoomId, ServerRoom(newRoomId, this)});
    //assert(success and iter->first == newRoomId);
    return { newRoomId, iter->second };
}

void Server::askForRoomDeletion(RoomId id) {
    std::cout << "(main server) room deletion requested\n";
    net::post(ioc, [&room=rooms.find(id)->second]{ room.interrupt(); });
    net::post(ioc, [&,id=id]{
        std::cout << "(async server) room deletion in progress...\n";
        rooms.erase(id);
        std::cout << "(async server) ...room deleted!\n";
#ifdef HTTP_CONTROLLED_SERVER
        if(id == 0)
            addRoom(0).second.launchGame({ mkEurope(), mkNearEast() }, 0);
#endif
    });
}

void Server::addSession(tcp::socket&& socket){
    auto session = new HttpSession(std::move(socket), *this);
    sessions.insert(session);
    session->run();
}

void Server::askForHttpSessionDeletion(HttpSession* session){
    sessions.erase(session);
    delete session;
}
