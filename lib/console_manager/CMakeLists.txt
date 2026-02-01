cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.h")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "source/*.cpp")

# Platform-specific sources
if(WIN32)
    list(APPEND SOURCES "platform/win/key_reader_windows.cpp")
else()
    list(APPEND SOURCES "platform/posix/key_reader_posix.cpp")
endif()

add_library(ConsoleManagerLib STATIC ${SOURCES} ${HEADERS})

set(INCLUDE_DIR include)
target_include_directories(ConsoleManagerLib PUBLIC ${INCLUDE_DIR})

target_link_libraries(ConsoleManagerLib PUBLIC UserInterfaceLib
                                               GameEngineLib
                                               BoardLib
                                               LogLib)
