cmake_minimum_required(VERSION 3.14)
project(DayTrender)

set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

file(GLOB DAYTRENDER_SRCS ./src/*.cpp ./src/api/*.cpp ./src/data/*.cpp ./src/interface/*.cpp)
file(COPY src/data/algodefs.h src/data/algotypes.h src/data/candle.h DESTINATION ../algorithms/include)

file(CREATE_LINK ../resources resources SYMBOLIC)
file(CREATE_LINK ../config config SYMBOLIC)
file(CREATE_LINK ../algorithms algorithms SYMBOLIC)

add_executable(daytrender ${DAYTRENDER_SRCS})

# setting properties
set_target_properties(daytrender PROPERTIES CXX_STANDARD 17)

if(UNIX)
	target_link_libraries(daytrender PRIVATE pthread dl)
endif()

# setting up nlohmann json
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# setting up fountain
add_library(fountain SHARED IMPORTED)
set_target_properties(fountain PROPERTIES IMPORTED_LOCATION ../lib/fountain/build/libfountain.so)
file(COPY lib/fountain/build/libfountain.so DESTINATION .)
target_include_directories(daytrender PRIVATE lib/fountain/build/include)

# setting up hxutils
add_library(hxutils SHARED IMPORTED)
set_target_properties(hxutils PROPERTIES IMPORTED_LOCATION ../lib/hxutils/build/libhxutils.so)
file(COPY lib/hxutils/build/libhxutils.so DESTINATION .)
target_include_directories(daytrender PRIVATE lib/hxutils/build/include)

# setting up cpphttplib
target_include_directories(daytrender PRIVATE lib/cpp-httplib/include)

# setting up pluginlib
target_include_directories(daytrender PRIVATE lib/pluginlib/build/include)

# linking daytrender libraries
target_link_libraries(daytrender PRIVATE fountain hxutils nlohmann_json OpenSSL::SSL OpenSSL::Crypto)
