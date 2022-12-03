list(APPEND files
	http_session.cpp
	listener.cpp
	room.cpp
	spectator.cpp
	server.cpp
	network_agent.cpp
)

add_library(network STATIC ${files})
target_link_libraries(network PRIVATE pthread agents)

add_library(network.server STATIC ${files} launch_game.cpp grammar.cpp grammar.json.cpp)
target_compile_definitions(network.server PUBLIC HTTP_CONTROLLED_SERVER HTTP_SERVE_FILES)
target_link_libraries(network.server PRIVATE pthread agents control search)

find_package( Boost REQUIRED )
