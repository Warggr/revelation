#ifndef REVELATION_SERVER_HPP
#define REVELATION_SERVER_HPP

#include "room.hpp"
#include "listener.hpp"
#include "semaphore.hpp"
#include <unordered_map>
#include <iostream>

class HttpSession;

/**
 * Class Server must be completely agnostic of whether the server is http-controlled or not.
 * If in the future, the server needs to know something about the dependent properties of Server_impl,
 * we can use a bridge pattern (every Server must contain a pointer to a Server_impl)
 */
class Server {
protected:
    net::io_context ioc; // The io_context is required for all I/O
    //ioc needs to be initialized before listener, that's why it comes first in the file
    Listener listener; // The Listener listens for new clients and adds them to the Rooms
    std::unordered_set<HttpSession*> sessions; //all these pointers are owning

public:
    Server(const char* ipAddress, unsigned short port);

    ~Server();

    void addSession(tcp::socket&& socket);

    void askForHttpSessionDeletion(HttpSession* session);

    template<typename Function>
    void async_do(Function&& fun){
        net::post(ioc, std::forward<Function>(fun));
    }
};

#ifdef HTTP_SERVE_FILES
#include "setup/units_repository.hpp"
#endif

class Server_impl: public Server {
    RoomId lastUsedIdentifier = 0;
    std::unordered_map<RoomId, ServerRoom_impl> rooms; //Each room contains a list of established WebSocket connections.
public:
#ifdef HTTP_SERVE_FILES
    const std::string doc_root;
#endif
#ifdef HTTP_CONTROLLED_SERVER
    UnitsRepository repo;
#endif

    Server_impl(const char* ipAddress, unsigned short port
#ifdef HTTP_SERVE_FILES
    , std::string_view doc_root
#endif
    )
    : Server(ipAddress, port)
#ifdef HTTP_SERVE_FILES
    , doc_root(doc_root)
#endif
    {
#ifdef HTTP_CONTROLLED_SERVER
        repo.mkDefaultTeams();
#endif
    };

    void start();

    void stop();

    void askForRoomDeletion(RoomId id);
    std::pair<RoomId, ServerRoom_impl&> addRoom(RoomId newRoomId);
    std::pair<RoomId, ServerRoom_impl&> addRoom(){ return addRoom(++lastUsedIdentifier); }
    std::unordered_map<RoomId, ServerRoom_impl>& getRooms() { return rooms; }
    const std::unordered_map<RoomId, ServerRoom_impl>& getRooms() const { return rooms; }
};

std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);

#endif //REVELATION_SERVER_HPP
