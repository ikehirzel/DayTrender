cmake_minimum_required(VERSION 3.9)
project(varlib)

add_library(var SHARED src/var.cpp)
add_library(var_static STATIC src/var.cpp)
add_library(var_staticpic STATIC src/var.cpp)

set_target_properties(var_staticpic PROPERTIES POSITION_INDEPENDENT_CODE true)

file(COPY src/var.h DESTINATION include/hirzel)
