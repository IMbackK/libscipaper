set(MODULE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../modapi ${CMAKE_CURRENT_SOURCE_DIR}/../scipaper)

add_library(test SHARED test.c)
target_link_libraries(test ${COMMON_LIBRARIES})
target_include_directories(test SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
target_include_directories(test PRIVATE ${MODULE_INCLUDE_DIRS})
set_target_properties(test PROPERTIES COMPILE_FLAGS ${COMMON_FLAGS})
install(TARGETS test DESTINATION ${SCI_MODULE_DIR})

add_library(crossref SHARED crossref.c)
target_link_libraries(crossref ${COMMON_LIBRARIES})
target_include_directories(crossref SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
target_include_directories(crossref PRIVATE ${MODULE_INCLUDE_DIRS})
set_target_properties(crossref PROPERTIES COMPILE_FLAGS ${COMMON_FLAGS})
install(TARGETS crossref DESTINATION ${SCI_MODULE_DIR})

add_library(core SHARED core)
target_link_libraries(core ${COMMON_LIBRARIES})
target_include_directories(core SYSTEM PRIVATE ${COMMON_INCLUDE_DIRS})
target_include_directories(core PRIVATE ${MODULE_INCLUDE_DIRS})
set_target_properties(core PROPERTIES COMPILE_FLAGS ${COMMON_FLAGS})
install(TARGETS core DESTINATION ${SCI_MODULE_DIR})

