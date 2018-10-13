find_package(Git REQUIRED)
include(ExternalProject)

enable_testing()

#ensure we have Catch2 downloaded
ExternalProject_Add(
    catch
    PREFIX ${CMAKE_BINARY_DIR}/external/catch
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    )

#add catch2 and its public interfaces
ExternalProject_Get_Property(catch source_dir)
set(CATCH_INCLUDE_DIR ${source_dir}/single_include 
    CACHE INTERNAL "Path to include folder for Catch")


add_library(Catch INTERFACE)
add_dependencies(Catch catch)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

# build and link our libraries
add_executable(AllocatorTest "${CMAKE_CURRENT_LIST_DIR}/AllocatorTest.cpp")
add_executable(IoManagerTest "${CMAKE_CURRENT_LIST_DIR}/IoManagerTest.cpp")
add_executable(ConfigManagerTest "${CMAKE_CURRENT_LIST_DIR}/ConfigManagerTest.cpp")
add_executable(ThreadPoolTest "${CMAKE_CURRENT_LIST_DIR}/ThreadPoolTest.cpp")
add_executable(LoggerTest "${CMAKE_CURRENT_LIST_DIR}/LoggerTest.cpp")
add_executable(ClockTest "${CMAKE_CURRENT_LIST_DIR}/ClockTest.cpp")


target_link_libraries(AllocatorTest Catch Threads::Threads s_util)
target_link_libraries(IoManagerTest Catch Threads::Threads s_util)
target_link_libraries(ConfigManagerTest Catch Threads::Threads s_util)
target_link_libraries(ThreadPoolTest Catch Threads::Threads s_util)
target_link_libraries(LoggerTest Catch s_util)
target_link_libraries(ClockTest Catch s_util)

