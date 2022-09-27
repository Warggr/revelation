#ifndef REVELATION_SERVER_HPP
#define REVELATION_SERVER_HPP

#include "room.hpp"
#include "listener.hpp"
#include "semaphore.hpp"
#include <unordered_map>
#include <iostream>

class HttpSession;

constexpr int TOTAL_AVAILABLE_ROOMS = 4;

class Server {
    RoomId lastUsedIdentifier = 0;
    net::io_context ioc; // The io_context is required for all I/O
    Listener listener; // The Listener listens for new clients and adds them to the Rooms
    Semaphore nbAvailableRooms { TOTAL_AVAILABLE_ROOMS };
    std::unordered_set<HttpSession*> sessions; //all these pointers are owning
public:
    std::unordered_map<RoomId, ServerRoom> rooms; //Each room contains a list of established WebSocket connections.

    Server(const char* ipAddress, unsigned short port);

    ~Server();

    std::pair<RoomId, ServerRoom&> addRoom();

    void askForRoomDeletion(RoomId id);

    void addSession(tcp::socket&& socket);

    void askForHttpSessionDeletion(HttpSession* session);

    void start();

    void stop();
};

#endif //REVELATION_SERVER_HPP
