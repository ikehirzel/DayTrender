cmake_minimum_required(VERSION 3.9)
project(logger)

file(COPY src/fountain.h lib/var/src/var.h DESTINATION include/hirzel)


include_directories(build/include)

# setting up shared build
add_library(fountain SHARED src/fountain.cpp lib/var/src/var.cpp)

# setting up static build
add_library(fountain_static STATIC src/fountain.cpp lib/var/src/var.cpp)

# setting up static pic build
add_library(fountain_staticpic STATIC src/fountain.cpp lib/var/src/var.cpp)
set_target_properties(fountain_static PROPERTIES POSITION_INDEPENDENT_CODE true)

