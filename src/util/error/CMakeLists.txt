
target_sources(s_util
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Logger.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/SentinelAssert.cpp"
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/Logger.h"
    "${CMAKE_CURRENT_LIST_DIR}/SentinelAssert.h"
    )

target_include_directories(s_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})
