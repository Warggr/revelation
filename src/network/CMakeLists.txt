add_library(network STATIC http_session.cpp listener.cpp room.cpp spectator.cpp server.cpp network_agent.cpp network_agent.hpp)

find_package( Boost REQUIRED )
