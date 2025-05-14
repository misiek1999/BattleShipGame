cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# Download spdlog from the Gerrit mirror (change GIT_TAG if you need a different version)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG f355b3d58f7067eee1706ff3c801c2361011f3d5    # Commit hash for v1.15.1
)

# FetchContent_MakeAvailable will download and add spdlog as a dependency.
FetchContent_MakeAvailable(spdlog)

# Define the active log level for spdlog.
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

# Add your executable (or library) target.
file(GLOB HEADERS "*.h" "*.hpp")
file(GLOB SOURCES "*.c" "*.cpp")

add_library(LogLib STATIC ${SOURCES} ${HEADERS})

target_include_directories(LogLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link spdlog to your target.
target_link_libraries(LogLib PUBLIC spdlog::spdlog)
