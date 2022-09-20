// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CPPCON2018_WEBSOCKET_SESSION_HPP
#define CPPCON2018_WEBSOCKET_SESSION_HPP

#include "semaphore.hpp"
#include "net.hpp"
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <queue>
#include <streambuf>

class ServerRoom;

namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;

using AgentId = unsigned short int;

/** Represents an active WebSocket connection to the server. */
class Spectator: public std::streambuf {
    /* The following two methods allow Spectator to be a kind of streambuf, so it can be used in std::ostreams */
    // This function will be called when trying "to sync the input and output streams". So it should dump the content.
    // Returns 0 on success, -1 on failure.
    int sync() override {
        if(pbase() == pptr()) return 0; //nothing to send
        send(std::make_shared<const std::string>(pbase(), pptr()));
        return std::streambuf::sync();
    };
    // Copied from https://bytes.com/topic/c/answers/62641-inheriting-streambuf.
    // This is called when new characters are added, but only 1 space is left in the buffer (so almost an overflow).
    std::streambuf::int_type overflow(std::streambuf::int_type c) override {
        if (!traits_type::eq_int_type(traits_type::eof(), c)) { //ignore this for the "end-of-file" character
            traits_type::assign(*pptr(), traits_type::to_char_type(c)); //write the character and advance by 1
            pbump(1);
        }
        return sync() == 0 ? traits_type::not_eof(c): traits_type::eof();
    }

    beast::flat_buffer buffer; //Only used for reading
    websocket::stream<tcp::socket> ws;
    std::queue<std::shared_ptr<const std::string>> writing_queue; //All messages that haven't been sent yet
    std::queue<std::string> reading_queue; //All messages that haven't been read yet
    Semaphore nb_messages_unread;
    ServerRoom& room;
    AgentId id; //Is 0 when we're not an agent, but only a spectator

    void on_accept(error_code ec);
    void on_write(error_code ec, std::size_t bytes_transferred);
    void on_read(error_code ec, std::size_t bytes_transferred);
    void fail(error_code ec, char const* what);
public:
    Spectator(tcp::socket socket, ServerRoom& room, AgentId id = 0);
    ~Spectator();

    //Accepts the websocket handshake asynchronously
    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req){
        ws.async_accept( req, [&](error_code ec){ on_accept(ec); } );
    }

    void send(const std::shared_ptr<const std::string>& message);

    std::string get();

    std::ostream asOStream(){
        return std::ostream(this);
    }
};

#endif
