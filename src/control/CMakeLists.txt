add_library(agents STATIC agent.cpp step_agent.cpp timeout.cpp)
target_link_libraries(agents PRIVATE gameplay)

add_library(control STATIC game.cpp)
target_link_libraries(control PRIVATE agents gameplay setup logging)
