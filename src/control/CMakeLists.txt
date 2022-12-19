add_library(base.agent STATIC agent.cpp step_agent.cpp timeout.cpp)
target_link_libraries(base.agent PRIVATE gameplay)

add_library(control STATIC game.cpp)
target_link_libraries(control PRIVATE base.agent gameplay base.logging)
