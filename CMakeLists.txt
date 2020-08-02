cmake_minimum_required(VERSION 3.0)

set(ANTA vector)
project(${ANTA})

add_executable(${ANTA} main.cpp)
target_compile_features(${ANTA} PRIVATE cxx_std_17)
