#include "network_logger.hpp"
#include "network/server.hpp"

LiveServerAndLogger::LiveServerAndLogger(GameRoom& serverRoom, std::string_view start): serverRoom(serverRoom){
    serverRoom.getServer()->async_do( [serverRoom=&serverRoom,cancel=cancellation_token,start=std::string(start)]{
        if(*cancel) return;
        serverRoom->setGreeterMessage(start);
    } );
}

void LiveServerAndLogger::addStep(std::string_view step) {
    serverRoom.getServer()->async_do( [serverRoom=&serverRoom,cancel=cancellation_token,step=std::string(step)]{
        if(*cancel) return;
        serverRoom->send(step);
    } );
}
