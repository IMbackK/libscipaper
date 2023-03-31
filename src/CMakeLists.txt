set(SRC_FILES
	sci-backend.c
	sci-conf.c
	sci-log.c
	sci-modules.c
	scipaper.c
	types.c
	utils.c
	nxjson.c)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${COMMON_LIBRARIES})
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${COMMON_FLAGS})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.cpp log.cpp)
set(LIBS_CLI -L. -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_cli ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_cli ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_cli ${LIBS_CLI} ${COMMON_LIBRARIES})
target_include_directories(${PROJECT_NAME}_cli SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME}_cli PROPERTIES COMPILE_FLAGS ${COMMON_FLAGS})
install(TARGETS ${PROJECT_NAME}_cli DESTINATION bin)

add_subdirectory(modules)
