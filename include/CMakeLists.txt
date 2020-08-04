set(LIB_NAME dl)
add_library(${LIB_NAME} INTERFACE)
target_sources(${LIB_NAME} INTERFACE
  vector.h
  compressed_pair.h)

target_include_directories(${LIB_NAME} INTERFACE .)
