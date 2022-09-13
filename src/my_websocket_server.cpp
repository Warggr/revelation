#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>

namespace net = boost::asio;
using namespace boost::beast;

net::io_context ioc;

class Server {
    websocket::stream<tcp_stream> websock;

    Server(websocket::stream<tcp_stream>&& websock): websock(std::move(websock)) {};
public:
    /* static Server clientFactory(const char* hostname) {
        // Outgoing connection: Resolve the hostname and bind the websocket to it
        websocket::stream<tcp_stream> ws(ioc); //the websocket
        net::ip::tcp::resolver resolver(ioc);
        get_lowest_layer(ws).connect(resolver.resolve(hostname, "ws"));

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        get_lowest_layer(ws).expires_never();

        ws.handshake(hostname, "/");
        return Server(std::move(ws));
    } */

    static Server serverFactory(unsigned short port) {
        //Incoming connection: Accept via an acceptor
        net::ip::tcp::acceptor acceptor(ioc);
        net::ip::tcp::endpoint endpoint(net::ip::tcp::v4(), port);
        acceptor.open(endpoint.protocol());
        acceptor.bind(endpoint);
        acceptor.listen();
        websocket::stream<tcp_stream> ws(acceptor.accept());

        get_lowest_layer(ws).expires_never();

        ws.accept();
        return Server(std::move(ws));
    }

    void send(std::string message){
        websock.text(true);
        websock.write(net::buffer(message));
    }

    void get(){
        flat_buffer buffer;
        websock.read(buffer);
        BOOST_ASSERT(websock.got_text());
        const auto data = buffer.data();
        std::cout << "Client says:" << std::string((char*)data.data(), data.size() ) << '\n';
        buffer.consume(buffer.size());
    }
};

void network_thread_function(Server* server){
    for(int i=0; i<20; i++)
        server->get();
}

int main(int argc, char* argv[]){
    auto const port = static_cast<unsigned short>(std::atoi(argv[1]));

    Server server = Server::serverFactory(port);
    std::thread network_thread(network_thread_function, &server);

    for(int i = 0; i<20; i++){
        std::string your_message;
        std::getline(std::cin, your_message);
        server.send(your_message);
    }

    network_thread.join();
    return 0;
}
