#ifndef REVELATION_SERVER_HPP
#define REVELATION_SERVER_HPP

#include "room.hpp"
#include "listener.hpp"
#include "semaphore.hpp"
#include <unordered_map>
#include <iostream>

class HttpSession;

class Server {
    RoomId lastUsedIdentifier = 0;
    net::io_context ioc; // The io_context is required for all I/O
    Listener listener; // The Listener listens for new clients and adds them to the Rooms
    std::unordered_set<HttpSession*> sessions; //all these pointers are owning
    std::pair<RoomId, ServerRoom&> addRoom(RoomId newRoomId);
public:
#ifdef HTTP_SERVE_FILES
    const std::string doc_root;
#endif
    std::unordered_map<RoomId, ServerRoom> rooms; //Each room contains a list of established WebSocket connections.

#ifdef HTTP_SERVE_FILES
    Server(const char* ipAddress, unsigned short port, std::string_view doc_root);
#else
    Server(const char* ipAddress, unsigned short port);
#endif

    ~Server();

    std::pair<RoomId, ServerRoom&> addRoom(){ return addRoom(++lastUsedIdentifier); }

    void askForRoomDeletion(RoomId id);

    void addSession(tcp::socket&& socket);

    void askForHttpSessionDeletion(HttpSession* session);

    void start();

    void stop();

    const std::unordered_map<RoomId, ServerRoom>& getRooms() const { return rooms; }
};

#endif //REVELATION_SERVER_HPP
