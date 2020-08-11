set(LIB_NAME dl)
add_library(${LIB_NAME} INTERFACE)
target_sources(${LIB_NAME} INTERFACE
  vector.h
  compressed_pair.h
  split_buffer.h
  type_utils.h)

target_include_directories(${LIB_NAME} INTERFACE .)
