// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#include "server.hpp"

Server::Server(const char* ipAddress, unsigned short port)
    : listener( *this, ioc, tcp::endpoint{net::ip::make_address(ipAddress), port} )
{
    std::cout << "(main) Starting server\n";
}

std::pair<RoomId, ServerRoom&> Server::addRoom() {
    std::cout << "(main) Add room to server\n";
    RoomId newRoomId = lastUsedIdentifier++;
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
        nbAvailableRooms.release();
    });
}
