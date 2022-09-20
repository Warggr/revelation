#ifndef REVELATION_SERVER_HPP
#define REVELATION_SERVER_HPP

#include "room.hpp"
#include "listener.hpp"
#include <unordered_map>

using RoomId = unsigned short int;

class Server {
    RoomId lastUsedIdentifier = 0;
    net::io_context ioc; // The io_context is required for all I/O
    Listener listener; // The Listener listens for new clients and adds them to the Rooms
public:
    std::unordered_map<RoomId, ServerRoom> rooms; //Each room contains a list of established WebSocket connections.

    Server(const char* ipAddress, unsigned short port);

    std::pair<RoomId, ServerRoom&> addRoom();

    void start(){
        //! this function runs indefinitely.
        //! To stop it, you need to call stop() (presumably from another thread)
        listener.listen();
        ioc.run();
    }

    void stop(){
        ioc.stop();
    }
};

#endif //REVELATION_SERVER_HPP
