cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set(CMAKE_CXX_STANDARD 17)

project(revelation VERSION ${PROJECT_VERSION} LANGUAGES CXX)

if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed! Create a build directory and run CMake from there." )
endif()

#options
option(LIBCPP "Build with libc++" FALSE)
option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" FALSE)
option(ENABLE_TESTING "Enable the building of the test" TRUE)
option(ENABLE_CPPCHECK "Enable testing with cppcheck" FALSE)
option(SIMPLE_BUILD "Build the project as minimally as possible" FALSE)
option(BUILD_DOC "Build the project's documentation" ON)
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
option(DEBUG_LOGGING "Enabling debug logging" FALSE)
option(ENABLE_PROFILING "Enabling profiling of code with gprof" FALSE)

set(SanitizeType "None" CACHE STRING "What sanitizers to use")
set_property(CACHE SanitizeType PROPERTY STRINGS "None" "adress" "thread" "undefined" "leak" "memory")

set(DEFAULT_BUILD_TYPE "Debug")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#add interfaces
add_library(project_warnings INTERFACE)
add_library(project_options INTERFACE)

if(NOT SanitizeType STREQUAL "None")
    target_compile_options(project_options INTERFACE -fsanitize=${SanitizeType})
    target_link_libraries(project_options INTERFACE -fsanitize=${SanitizeType})
endif()

if(ONLY_COVERAGE OR ENABLE_COVERAGE)
    target_compile_options(project_options INTERFACE --coverage -O0 -g)
    target_link_libraries(project_options INTERFACE --coverage)
endif()

if(ENABLE_PROFILING)
    target_compile_options(project_options INTERFACE -pg)
endif()

#Warnings
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Werror
        -Wnon-virtual-dtor -Wcast-align
        -Wunused -Woverloaded-virtual -Wpedantic
        -Wdouble-promotion
        -Wformat=2)

target_compile_options(project_options INTERFACE
    -fPIC
)

# some GCC specific warnings. These flags are added only if the used compiler is GCC.
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(project_warnings INTERFACE
            -Wmisleading-indentation
            -Wduplicated-cond
            -Wlogical-op
            -Wuseless-cast )
    target_link_libraries(project_options INTERFACE stdc++fs)
endif()

#colored output
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options (project_options INTERFACE -fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options (project_options INTERFACE -fcolor-diagnostics)
    endif()
endif()

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK
                ${CPPCHECK}
                --suppress=syntaxError
                --enable=all
                --inconclusive)
    else()
        message(SEND_ERROR "cppcheck requested but executable not found")
    endif()
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
    else()
        message(SEND_ERROR "clang-tidy requested but executable not found")
    endif()
endif()

#json serializer
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)
include_directories(PRIVATE ${CMAKE_BINARY_DIR}/_deps/json-src/include)

# Lua wrapper
FetchContent_Declare(sol2 URL https://github.com/ThePhD/sol2/archive/refs/tags/v3.3.0.tar.gz)
FetchContent_MakeAvailable(sol2)
include_directories(PRIVATE ${CMAKE_BINARY_DIR}/_deps/sol2-src/include)

find_package( Boost 1.40 COMPONENTS program_options python REQUIRED )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

link_libraries(project_warnings project_options)

add_subdirectory(src)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
        "A flag showing that CMake has configured at least once.")
