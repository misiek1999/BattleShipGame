cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp")


add_library(GameSessionLib STATIC ${SOURCES} ${HEADERS})

set(INCLUDE_DIR include)
target_include_directories(GameSessionLib PUBLIC ${INCLUDE_DIR})

target_link_libraries(GameSessionLib PUBLIC GameEngineLib PlayerInterfaceLib BoardLib PlayerTypeLib LogLib)
