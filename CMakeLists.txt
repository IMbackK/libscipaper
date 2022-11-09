cmake_minimum_required(VERSION 3.20)

project(scipaper LANGUAGES C)
set(SCI_SYSCONF_DIR /usr/share/scipaper)
set(SCI_MODULE_DIR /usr/lib/scipaper/modules)

add_definitions(-D_GNU_SOURCE)
add_definitions(-DSCI_MODULE_DIR=${SCI_MODULE_DIR})
add_definitions(-DSCI_SYSCONF_DIR=${SCI_SYSCONF_DIR})
add_definitions(-DSCI_SYSCONF_INI=scipaper.ini)

find_package(Doxygen)

find_package(PkgConfig REQUIRED)
find_package(LibXml2)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)
pkg_check_modules(CURL REQUIRED libcurl)

set(COMMON_INCLUDE_DIRS
  ${GLIB_INCLUDE_DIRS}
  ${GMODULE_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_LIST_DIR}/src/
  ${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT_NAME}
  ${CMAKE_CURRENT_LIST_DIR}/src/modapi)
message(${COMMON_INCLUDE_DIRS})
set(COMMON_LIBRARIES ${GLIB_LIBRARIES} ${GMODULE_LIBRARIES} ${CURL_LIBRARIES})
set(COMMON_FLAGS "-std=c99 -Wall -O2 -march=native -g -fno-strict-aliasing")

set(CMAKE_INSTALL_PREFIX "/usr")

set(API_HEADERS_DIR src/scipaper/)
set(API_HEADERS
	${API_HEADERS_DIR}/scipaper.h
	${API_HEADERS_DIR}/types.h
)
install(FILES ${API_HEADERS} DESTINATION include/${PROJECT_NAME})

if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/libscipaper.doxygen.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/doc/libscipaper.doxygen)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen needs to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

add_subdirectory(config)
add_subdirectory(src)
