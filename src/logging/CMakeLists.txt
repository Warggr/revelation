add_library(base.logging STATIC
	step.cpp
	json.cpp
	logger.cpp
	file_logger.cpp
)

add_library(network.logging STATIC
		network_logger.cpp
)
target_link_libraries(network.logging PRIVATE network)
