#include "network/server_impl.hpp"
#include <iostream>
#include <filesystem>

int main(int nbargs, const char** args){
    if(nbargs != 4){
        std::cerr << "Usage: " << args[0] << " <IPaddr> <port> <doc_root>\n";
        exit(1);
    }
    std::filesystem::current_path(std::filesystem::current_path().parent_path() / "resources");
    const char* ipAddr = args[1];
    unsigned short int port = std::atoi(args[2]);
    const char* doc_root = args[3];
    Server_impl server(ipAddr, port, doc_root);

    server.start();
}
