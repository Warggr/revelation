// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef CPPCON2018_HTTP_SESSION_HPP
#define CPPCON2018_HTTP_SESSION_HPP

#include "net.hpp"
#include "connection_list.hpp"
#include <boost/beast.hpp>
#include <memory>

namespace beast = boost::beast;
namespace http = boost::beast::http;

/** Represents an established HTTP connection. */
class HttpSession {
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    ConnectionList& server;

    void fail(error_code ec, char const* what);
    void on_read(error_code ec, std::size_t);
    void on_write(error_code ec, std::size_t, bool close);
    explicit HttpSession(tcp::socket&& socket, ConnectionList& server);
    void run();
public:
    static void start(tcp::socket&& socket, ConnectionList& server){
        //Sessions delete themselves when they don't have any tasks anymore.
        auto* session = new HttpSession(std::move(socket), server);
        session->run();
    }
};

#endif
