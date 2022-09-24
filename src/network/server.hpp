#ifndef REVELATION_SERVER_HPP
#define REVELATION_SERVER_HPP

#include "room.hpp"
#include "listener.hpp"
#include "semaphore.hpp"
#include <unordered_map>
#include <iostream>

constexpr int TOTAL_AVAILABLE_ROOMS = 4;

class Server {
    RoomId lastUsedIdentifier = 0;
    net::io_context ioc; // The io_context is required for all I/O
    Listener listener; // The Listener listens for new clients and adds them to the Rooms
    Semaphore nbAvailableRooms { TOTAL_AVAILABLE_ROOMS };
public:
    std::unordered_map<RoomId, ServerRoom> rooms; //Each room contains a list of established WebSocket connections.

    Server(const char* ipAddress, unsigned short port);

    std::pair<RoomId, ServerRoom&> addRoom();

    void askForRoomDeletion(RoomId id);

    void start(){
        std::cout << "(network) Starting server...\n";
        //! this function runs indefinitely.
        //! To stop it, you need to call stop() (presumably from another thread)
        listener.listen();
        ioc.run();
    }

    void stop(){
        std::cout << "(network) ...stopping server and interrupting rooms\n";
        for(auto& [id, room] : rooms){
            room.interrupt();
        }
        nbAvailableRooms.acquire(TOTAL_AVAILABLE_ROOMS);
        ioc.stop();
    }
};

#endif //REVELATION_SERVER_HPP
