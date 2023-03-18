add_library(base.agent STATIC agent.cpp step_agent.cpp timeout.cpp)
target_link_libraries(base.agent PRIVATE gameplay)

add_library(python.agent STATIC py_agent.cpp)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
target_include_directories(python.agent PUBLIC ${Python3_INCLUDE_DIRS})
target_link_libraries(python.agent PRIVATE base.agent ${Boost_LIBRARIES} ${Python3_LIBRARIES})

add_library(control STATIC game.cpp)
target_link_libraries(control PRIVATE base.agent gameplay base.logging)
