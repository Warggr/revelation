// Adapted from https://github.com/vinniefalco/CppCon2018
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef CPPCON2018_ASIO_HPP
#define CPPCON2018_ASIO_HPP

#include <boost/asio.hpp>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using error_code = boost::system::error_code;

#endif
