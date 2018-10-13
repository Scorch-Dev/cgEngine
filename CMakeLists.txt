cmake_minimum_required(VERSION 3.9)

project(Sentinel)
set(SENTINEL_VERSION_MAJOR 0)
set(SENTINEL_VERSION_MINOR 1)
set(SENTINEL_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_LIST_DIR}/src/CMakeLists.txt)

#Sentinel Testing optional
option(S_BUILD_TESTS "Determines if we build tests (default: on)" ON)

if(S_BUILD_TESTS)
    include(${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt)
endif()