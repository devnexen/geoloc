cmake_minimum_required(VERSION 3.0)
add_custom_target(yacc_command COMMAND yacc -o geolocd/y.tab.c geolocd/parse.y)
add_subdirectory(geolocd)
add_subdirectory(geolocctl)
