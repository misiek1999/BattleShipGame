cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")


add_library(BoardLib STATIC ${SOURCES} ${HEADERS})

set(INCLUDE_DIR include)
target_include_directories(BoardLib PUBLIC ${INCLUDE_DIR})

target_link_libraries(BoardLib PUBLIC PlayerTypeLib LogLib)
