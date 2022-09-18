add_library(network STATIC http_session.cpp listener.cpp connection_list.cpp spectator.cpp server.cpp)

find_package( Boost REQUIRED )
