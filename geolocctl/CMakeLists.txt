include_directories(../geolocd)
add_executable(geolocctl geolocctl.c ../geolocd/y.tab.c ../geolocd/log.c)
add_dependencies(geolocctl geolocd)
