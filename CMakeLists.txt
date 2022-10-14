cmake_minimum_required(VERSION 3.20)

project(scipaper LANGUAGES C)
set(SCI_SYSCONF_DIR /usr/share/scipaper)
set(SCI_MODULE_DIR /usr/lib/scipaper/modules)

add_definitions(-D_GNU_SOURCE)
add_definitions(-DSCI_MODULE_DIR=${SCI_MODULE_DIR})
add_definitions(-DSCI_SYSCONF_DIR=${SCI_SYSCONF_DIR})
add_definitions(-DSCI_SYSCONF_INI=scipaper.ini)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)

set(COMMON_INCLUDE_DIRS ${GLIB_INCLUDE_DIRS} ${GMODULE_INCLUDE_DIRS} ./${PROJECT_NAME} ./modapi .)
set(COMMON_LIBRARIES ${GLIB_LIBRARIES} ${GMODULE_LIBRARIES})

set(CMAKE_INSTALL_PREFIX "/usr")

set(API_HEADERS_DIR src/scipaper/)
set(API_HEADERS
	${API_HEADERS_DIR}/scipaper.h
)
install(FILES ${API_HEADERS} DESTINATION include/${PROJECT_NAME})

add_subdirectory(config)
add_subdirectory(src)
