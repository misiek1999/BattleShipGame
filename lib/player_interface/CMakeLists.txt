cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADERS "include/*.h")


add_library(PlayerInterfaceLib STATIC ${HEADERS})

set(INCLUDE_DIR include)
target_include_directories(PlayerInterfaceLib PUBLIC ${INCLUDE_DIR})

target_link_libraries(PlayerInterfaceLib PUBLIC GameSessionLib
                                       PlayerTypeLib
                                       BoardLib
                                       GameTypesLib
                                       LogLib)
