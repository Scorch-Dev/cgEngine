target_sources(s_util
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/SVar.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/ConfigManager.cpp"
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/SVar.h"
    "${CMAKE_CURRENT_LIST_DIR}/ConfigManager.h"
    )

target_include_directories(s_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})
