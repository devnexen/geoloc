cmake_minimum_required(VERSION 3.0)
find_library(GEOIP_LIB GeoIP)
find_path(GEOIP_INC GeoIP.h)


file(GLOB DSRCS geolocd/*.c geolocd/modules/*.c)
file(GLOB CTLSRCS geolocctl/*.c geolocd/log.c)

set(CTLSRCS ${CTLSRCS} geolocd/y.tab.c)

include_directories(geolocd)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
add_executable(geolocd ${DSRCS})
target_link_libraries(geolocd ${GEOIP_LIB})
add_custom_command(TARGET geolocd PRE_BUILD COMMAND yacc -o y.tab.c parse.y WORKING_DIRECTORY geolocd)
add_executable(geolocctl ${CTLSRCS})
add_dependencies(geolocctl geolocd)

if(GEOLOC_INSTALL_PATH)
    install(TARGETS geolocd geolocctl DESTINATION ${GEOLOC_INSTALL_PATH}/sbin)
endif()
