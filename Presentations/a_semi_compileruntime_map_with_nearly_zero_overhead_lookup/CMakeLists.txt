cmake_minimum_required (VERSION 2.8)

project (semimap)

set(CMAKE_CXX_FLAGS_RELEASE -Ofast -march=native)

add_executable(semimap_test test.cpp)

# Set C++ standard for all targets
set_property(TARGET semimap_test PROPERTY CXX_STANDARD 17)
