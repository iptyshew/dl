cmake_minimum_required(VERSION 3.0)

set(ANTA vector)
project(${ANTA})
add_executable(${ANTA} main.cpp)
target_compile_features(${ANTA} PRIVATE cxx_std_17)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
