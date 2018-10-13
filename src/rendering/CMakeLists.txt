
target_sources(sentinel
	PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/Window.cpp"
	PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/Window.h"
	)

target_include_directories(sentinel PUBLIC ${CMAKE_CURRENT_LIST_DIR})