cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE HEADERS "include/*.h")

add_library(GameTypesLib STATIC ${HEADERS})

set(INCLUDE_DIR include)
target_include_directories(GameTypesLib PUBLIC ${INCLUDE_DIR})

set_target_properties(GameTypesLib PROPERTIES LINKER_LANGUAGE CXX)
