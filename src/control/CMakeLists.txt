add_library(control STATIC game.cpp)
target_link_libraries(control PRIVATE agents gameplay setup logging)

add_library(agents STATIC agent.cpp)
