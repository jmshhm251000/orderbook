cmake_minimum_required(VERSION 3.20)
project(orderbook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Download GoogleTest (pinned version)
include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Library: every .cpp in src/ (no main() here)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_library(orderbook ${SRC_FILES})
target_include_directories(orderbook PUBLIC include)

# App: your own main()
add_executable(app apps/main.cpp)
target_link_libraries(app PRIVATE orderbook)

# Tests: every .cpp in tests/ (gtest_main provides main())
enable_testing()
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS tests/*.cpp)
add_executable(tests ${TEST_FILES})
target_link_libraries(tests PRIVATE orderbook GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(tests)
