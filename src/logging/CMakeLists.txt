add_library(print STATIC
	json.cpp
	print.cpp
)

add_library(base.logging STATIC
	step.cpp
	logger.cpp
	file_logger.cpp
)
target_link_libraries(base.logging PRIVATE print)

add_library(network.logging STATIC
		network_logger.cpp
)
target_link_libraries(network.logging PRIVATE network)
