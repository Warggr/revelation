add_executable(revelation main.cpp)
add_executable(revelation-server main.server.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(control)
add_subdirectory(gameplay)
add_subdirectory(logging)
add_subdirectory(network)
add_subdirectory(search)
add_subdirectory(setup)
target_link_libraries(revelation PRIVATE setup control network search.agent network.logging ${Boost_LIBRARIES})
target_link_libraries(revelation-server PRIVATE server.network)
