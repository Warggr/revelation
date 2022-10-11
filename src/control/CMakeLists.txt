add_library(agents STATIC agent.cpp)
target_link_libraries(agents PRIVATE gameplay)

add_library(control STATIC game.cpp)
target_link_libraries(control PRIVATE agents gameplay setup logging)
