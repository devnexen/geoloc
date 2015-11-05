include_directories(../geolocd)
file(GLOB SRCS *.c ../geolocd/log.c ../geolocd/y.tab.c)
add_executable(geolocctl ${SRCS})
add_dependencies(geolocctl geolocd)

if (GEOLOC_INSTALL_PATH)
	install(TARGETS geolocctl DESTINATION ${GEOLOC_INSTALL_PATH}/sbin)
endif()
