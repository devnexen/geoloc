cmake_minimum_required(VERSION 3.0)
find_library(GEOIP_LIB GeoIP)
find_path(GEOIP_INC GeoIP.h)


file(GLOB DSRCS geolocd/*.c geolocd/modules/*.c)
file(GLOB CTLSRCS geolocctl/*.c geolocd/log.c geolocd/y*.c)
set_source_files_properties(geolocd/geoloc.c PROPERTIES OBJECT_DEPENDS geolocd/y.tab.c)

set(CTLSRCS ${CTLSRCS})

include_directories(geolocd)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
add_executable(geolocd ${DSRCS})
target_link_libraries(geolocd ${GEOIP_LIB})
add_executable(geolocctl ${CTLSRCS})
add_dependencies(geolocctl geolocd)

if(GEOLOC_INSTALL_PATH)
    install(TARGETS geolocd geolocctl DESTINATION ${GEOLOC_INSTALL_PATH}/sbin)
endif()
