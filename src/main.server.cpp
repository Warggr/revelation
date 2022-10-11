#include "network/server.hpp"
#include <iostream>

int main(int nbargs, const char** args){
    if(nbargs != 4){
        std::cerr << "Usage: " << args[0] << " <IPaddr> <port> <doc_root>\n";
        exit(1);
    }
    const char* ipAddr = args[1];
    unsigned short int port = std::atoi(args[2]);
    const char* doc_root = args[3];
    Server_HTTPFileServer server(ipAddr, port, doc_root);

    server.start();
}
