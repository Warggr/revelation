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
    std::pair<RoomId, ServerRoom_impl&> addRoom(RoomId newRoomId);
public:
    std::unordered_map<RoomId, ServerRoom_impl> rooms; //Each room contains a list of established WebSocket connections.

    Server(const char* ipAddress, unsigned short port);

    ~Server();

    std::pair<RoomId, ServerRoom_impl&> addRoom(){ return addRoom(++lastUsedIdentifier); }

    void askForRoomDeletion(RoomId id);

    void addSession(tcp::socket&& socket);

    void askForHttpSessionDeletion(HttpSession* session);

    void start();

    void stop();

    const std::unordered_map<RoomId, ServerRoom_impl>& getRooms() const { return rooms; }
};

#ifdef HTTP_SERVE_FILES
class Server_HTTPFileServer: public Server {
public:
    const std::string doc_root;

    Server_HTTPFileServer(const char* ipAddress, unsigned short port, std::string_view doc_root)
    : Server(ipAddress, port), doc_root(doc_root)
    {
    }
    ;
};
using Server_impl = Server_HTTPFileServer;
#else
using Server_impl = Server;
#endif

#endif //REVELATION_SERVER_HPP
