add_library(logging STATIC
	step.cpp
	json.cpp
	logger.cpp
)

target_link_libraries(logging PRIVATE network)
