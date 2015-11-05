include_directories(../geolocd)
add_executable(geolocctl geolocctl.c ../geolocd/y.tab.c ../geolocd/log.c)
add_dependencies(geolocctl geolocd)

if (GEOLOC_INSTALL_PATH)
	install(TARGETS geolocctl DESTINATION ${GEOLOC_INSTALL_PATH}/sbin)
endif()
