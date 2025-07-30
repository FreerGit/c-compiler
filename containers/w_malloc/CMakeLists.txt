cmake_minimum_required(VERSION 3.14)
project(cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Catch2 using FetchContent
include(FetchContent)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.0 # or latest
)
FetchContent_MakeAvailable(catch2)

enable_testing()

# Add test executable
add_executable(container_tests
    test/vector.cpp
)

target_include_directories(container_tests PRIVATE .)
target_link_libraries(container_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(container_tests)
