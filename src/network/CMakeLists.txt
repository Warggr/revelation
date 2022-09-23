add_library(network STATIC
	http_session.cpp
	listener.cpp
	room.cpp
	spectator.cpp
	server.cpp
	network_agent.cpp
)
target_link_libraries(network PRIVATE pthread agents)

find_package( Boost REQUIRED )
