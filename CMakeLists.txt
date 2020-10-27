cmake_minimum_required(VERSION 3.9)
project(DayTrender)

file(GLOB DAYTRENDER_SRCS ./src/*.cpp ./src/api/*.cpp ./src/data/*.cpp)

add_executable(daytrender ${DAYTRENDER_SRCS})
add_executable(dtbuild ./src/dtbuild/dtbuild.cpp)

# setting properties
set_target_properties(daytrender PROPERTIES CXX_STANDARD 17)
set_target_properties(dtbuild PROPERTIES CXX_STANDARD 17)

if(UNIX)
	target_link_libraries(daytrender PRIVATE pthread dl)
	target_link_options(daytrender PRIVATE "-Wl,-rpath='$ORIGIN'")
endif()

# setting up nlohmann json
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# setting up fountain
add_library(fountain SHARED IMPORTED)
set_target_properties(fountain PROPERTIES IMPORTED_LOCATION ../lib/fountain/build/libfountain.so)
target_include_directories(daytrender PRIVATE lib/fountain/build/include)

# setting up hxutils
add_library(hxutils SHARED IMPORTED)
set_target_properties(hxutils PROPERTIES IMPORTED_LOCATION ../lib/hxutils/build/libhxutils.so)
target_include_directories(daytrender PRIVATE lib/hxutils/build/include)
target_include_directories(dtbuild PRIVATE lib/hxutils/build/include)

# setting up cpphttplib
target_include_directories(daytrender PRIVATE lib/cpp-httplib/include)

# setting up tinyplug
target_include_directories(daytrender PRIVATE lib/tinyplug/include)

# linking daytrender libraries
target_link_libraries(daytrender PRIVATE fountain hxutils nlohmann_json OpenSSL::SSL OpenSSL::Crypto)

# linking dtbuild libraries
target_link_libraries(dtbuild PRIVATE hxutils)

# copying res folder
file(COPY res DESTINATION .)
