project(test_dl)


set(${PROJECT_NAME}_SRC
  vector_test.cpp
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ../include)

add_test(NAME ${PROJECT_NAME}
		 COMMAND ${PROJECT_NAME})
