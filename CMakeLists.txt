cmake_minimum_required(VERSION 3.0)

if (EXISTS "geolocd/y.tab.c")
	add_custom_target(yacc_clean COMMAND rm geolocd/y.tab.c)
else()
	add_custom_target(yacc_clean COMMAND echo \"\")
endif()

add_custom_target(yacc_command COMMAND yacc -o geolocd/y.tab.c geolocd/parse.y)
add_dependencies(yacc_command yacc_clean)
subdirs(geolocd)
subdirs(geolocctl)
