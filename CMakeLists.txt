cmake_minimum_required(VERSION 3.0)
find_library(GEOIP_LIB GeoIP)
find_path(GEOIP_INC GeoIP.h)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Checking libbsd for Linux")
    find_library(BSD_LIB bsd)
    add_definitions(-DHAVE_NO_BSDFUNCS)
    message(STATUS "${BSD_LIB}")
endif()

file(GLOB DSRCS geolocd/*.c geolocd/modules/*.c)
file(GLOB CTLSRCS geolocctl/*.c geolocd/log.c geolocd/y*.c)

set(CTLSRCS ${CTLSRCS})

include_directories(geolocd)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
add_executable(geolocd ${DSRCS})
target_link_libraries(geolocd ${GEOIP_LIB} ${BSD_LIB})
add_executable(geolocctl ${CTLSRCS})
target_link_libraries(geolocctl ${BSD_LIB})
add_dependencies(geolocctl geolocd)

if(GEOLOC_INSTALL_PATH)
    install(TARGETS geolocd geolocctl DESTINATION ${GEOLOC_INSTALL_PATH}/sbin)
endif()
