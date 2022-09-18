#ifndef REVELATION_SERVER_HPP
#define REVELATION_SERVER_HPP

#include "connection_list.hpp"
#include "listener.hpp"

class Server {
public:
    net::io_context ioc; // The io_context is required for all I/O
    ConnectionList connections; //The Server contains all established WebSocket connections.
    Listener listener; // The Listener listens for new clients and adds them to the Server

    Server(const char* ipAddress, unsigned short port, const std::string& message);

    void start(){
        //! this function runs indefinitely.
        //! To stop it, you need to call stop() (presumably from another thread)
        listener.listen();
        ioc.run();
    }

    void stop(){
        ioc.stop();
    }

    void send(std::string&& message){
        connections.send(message);
    }
};

#endif //REVELATION_SERVER_HPP
