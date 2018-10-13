
#optionally enable assertions in config
option(ASSERTIONS_ENABLED "Determines if assertions are enabled (default: on)" ON)

configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/SentinelConfig.h.in"
    "${CMAKE_CURRENT_LIST_DIR}/SentinelConfig.h"
	@ONLY
    )

find_package(SDL2 REQUIRED HINTS ${CMAKE_SOURCE_DIR}/lib/SDL2)
include_directories(${SDL2_INCLUDE_DIRS})
	

#Other package deps go here

#add sentinel executable + sources, then link
add_executable(sentinel "${CMAKE_CURRENT_LIST_DIR}/main.cpp")

include(${CMAKE_CURRENT_LIST_DIR}/util/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/hid/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/rendering/CMakeLists.txt)

target_link_libraries(sentinel ${SDL2_LIBRARIES})