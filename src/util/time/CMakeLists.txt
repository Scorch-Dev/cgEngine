
target_sources(s_util
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Clock.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Timer.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/Clock.h"
	"${CMAKE_CURRENT_LIST_DIR}/Timer.h"
    )

target_include_directories(s_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})