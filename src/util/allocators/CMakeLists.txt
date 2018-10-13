
target_sources(s_util
    PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/MemoryPool.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/MemoryStack.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/DbFrameAllocator.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/ThreadPool.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/StringId.cpp"
    PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/MemoryPool.h"
    "${CMAKE_CURRENT_LIST_DIR}/MemoryStack.h"
    "${CMAKE_CURRENT_LIST_DIR}/DbFrameAllocator.h"
	"${CMAKE_CURRENT_LIST_DIR}/ThreadPool.h"
	"${CMAKE_CURRENT_LIST_DIR}/StringId.h"
    )

target_include_directories(s_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})
