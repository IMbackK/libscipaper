set(MODULE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../modapi ${CMAKE_CURRENT_SOURCE_DIR}/../scipaper)

add_library(test SHARED test.c)
target_link_libraries(test ${COMMON_LIBRARIES})
target_include_directories(test SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
target_include_directories(test PRIVATE ${MODULE_INCLUDE_DIRS})
install(TARGETS test DESTINATION ${SCI_MODULE_DIR})

