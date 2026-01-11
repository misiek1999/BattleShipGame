cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE SOURCES "source/*.cpp")


add_library(PlayerManagerLib STATIC ${SOURCES} ${HEADERS})

set(INCLUDE_DIR include)
target_include_directories(PlayerManagerLib PUBLIC ${INCLUDE_DIR})

target_link_libraries(PlayerManagerLib PUBLIC PlayerBotLib GameSessionLib
                                              GameEngineLib PlayerInterfaceLib BoardLib PlayerTypeLib LogLib)
