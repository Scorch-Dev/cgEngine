
add_library(s_util "")

set(PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_sources(s_util
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/IoManager.cpp"
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/MacroOverload.h"
    "${CMAKE_CURRENT_LIST_DIR}/IoManager.h"
    )

include("${CMAKE_CURRENT_LIST_DIR}/allocators/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/config/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/error/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/time/CMakeLists.txt")

#set_target_properties(s_util PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)
target_link_libraries(s_util Threads::Threads)
target_include_directories(s_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})
