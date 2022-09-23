#include "network/server.hpp"
#include <iostream>

int main(int nbargs, const char** args){
    if(nbargs != 3){
        std::cerr << "Usage: " << args[0] << " <IPaddr> <port>\n";
        exit(1);
    }
    const char* ipAddr = args[1];
    unsigned short int port = std::atoi(args[2]);
    Server server(ipAddr, port);

    server.start();
}
