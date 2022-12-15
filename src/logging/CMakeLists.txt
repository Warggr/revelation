add_library(logging STATIC
	step.cpp
	json.cpp
	logger.cpp
	file_logger.cpp
)

add_library(network_logger STATIC
		network_logger.cpp
)
target_link_libraries(network_logger PRIVATE network logging)
