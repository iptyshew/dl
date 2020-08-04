project(test_dl)

set(${PROJECT_NAME}_SRC
  vector_test.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ../include)
target_link_libraries(${PROJECT_NAME} dl ${CONAN_LIBS})

add_test(NAME ${PROJECT_NAME}
		 COMMAND ${PROJECT_NAME})
