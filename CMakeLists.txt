cmake_minimum_required(VERSION 3.0)
find_library(GEOIP_LIB GeoIP)
find_path(GEOIP_INC GeoIP.h)

file(GLOB DSRCS geolocd/*.c geolocd/modules/*.c)
file(GLOB CTLSRCS geolocctl/*.c geolocd/log.c)

if(EXISTS "geolocd/y.tab.c")
	add_custom_target(yacc_clean COMMAND rm geolocd/y.tab.c)
else()
	add_custom_target(yacc_clean COMMAND echo \"\")
endif()

add_custom_target(yacc_command COMMAND yacc -o geolocd/y.tab.c geolocd/parse.y)
add_dependencies(yacc_command yacc_clean)
set(DSRCS ${DSRCS} geolocd/y.tab.c)
set(CTLSRCS ${CTLSRCS} geolocd/y.tab.c)

include_directories(geolocd)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
add_executable(geolocd ${DSRCS})
target_link_libraries(geolocd ${GEOIP_LIB})
add_dependencies(geolocd yacc_command)
add_executable(geolocctl ${CTLSRCS})
add_dependencies(geolocctl geolocd)

if(GEOLOC_INSTALL_PATH)
    install(TARGETS geolocd geolocctl DESTINATION ${GEOLOC_INSTALL_PATH}/sbin)
endif()
