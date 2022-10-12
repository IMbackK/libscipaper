cmake_minimum_required(VERSION 3.20)

project(scipaper LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)

set(COMMON_INCLUDE_DIRS ${GLIB_INCLUDE_DIRS} ${GMODULE_INCLUDE_DIRS})
set(COMMON_LIBRARIES ${GLIB_LIBRARIES} ${GMODULE_LIBRARIES}

set(SRC_FILES)

set(API_HEADERS_DIR scipaper/)
set(API_HEADERS
	${API_HEADERS_DIR}/scipaper.h
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${COMMON_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC eisgenerator)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-std=c++20 -Wall -O2 -march=native -g" LINK_FLAGS "-flto")

set(CMAKE_INSTALL_PREFIX "/usr")
install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${API_HEADERS} DESTINATION include/${PROJECT_NAME})

link_directories(${CMAKE_CURRENT_BINARY_DIR})
set(SRC_FILES_TEST_APP main.c)
set(LIBS_CLI -L. -l${PROJECT_NAME})
add_executable(${PROJECT_NAME}_cli ${SRC_FILES_TEST_APP})
add_dependencies(${PROJECT_NAME}_cli ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}_cli ${LIBS_CLI} ${COMMON_LIBRARIES})
target_include_directories(${PROJECT_NAME}_cli PUBLIC scipaper)
target_include_directories(${PROJECT_NAME}_cli SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME}_cli PROPERTIES COMPILE_FLAGS "-std=c++20 -Wall -O2 -march=native -g" LINK_FLAGS "-flto")
install(TARGETS ${PROJECT_NAME}_cli DESTINATION bin)
