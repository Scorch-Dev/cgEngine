
target_sources(sentinel
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/InputManager.cpp"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/InputManager.h"
	)

target_include_directories(sentinel PUBLIC ${CMAKE_CURRENT_LIST_DIR})