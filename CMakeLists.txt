cmake_minimum_required(VERSION 3.14)
project("DayTrender")

set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

# globbing sources for daytrender

file(GLOB DAYTRENDER_SRCS src/*.cpp src/api/*.cpp src/data/*.cpp src/interface/*.cpp)
file(GLOB STRATEGY_TYPES_SRCS src/data/strategydata.cpp src/data/indicator.cpp src/data/candle.cpp)
file(GLOB CLIENT_TYPES_SRCS src/data/assetinfo.cpp src/data/accountinfo.cpp src/data/candle.cpp)

# creating symlinks so files can be shared between build folder and project folder

file(CREATE_LINK ../clients.json clients.json SYMBOLIC)
file(CREATE_LINK ../server.json server.json SYMBOLIC)
file(CREATE_LINK ../strategies strategies SYMBOLIC)
file(CREATE_LINK ../clients clients SYMBOLIC)
file(CREATE_LINK ../src/interface/webinterface.html webinterface.html SYMBOLIC)

# creating main executable of project
add_executable(daytrender ${DAYTRENDER_SRCS})

# stops cmake from prepending lib before plugin names
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# compiling each strategies plugin
file(GLOB STRAT_SRCS "ext/strategies/*.cpp")

foreach(STRAT ${STRAT_SRCS})
	# get filename without folder or extension
	get_filename_component(FILENAME ${STRAT} NAME_WE)
	# create shared object for it
	add_library(${FILENAME} SHARED ${STRAT} ${STRATEGY_TYPES_SRCS})
	# tell it to go to strategies folder
	set_target_properties(${FILENAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/strategies)
	target_include_directories(${FILENAME} PRIVATE src/data src/api)
endforeach()

# compiling each client plugin
file(GLOB CLIENT_SRCS "ext/clients/*.cpp")

foreach(CLIENT ${CLIENT_SRCS})
	# get filename without folder or extension
	get_filename_component(FILENAME ${CLIENT} NAME_WE)
	# create shared object for it
	add_library(${FILENAME} SHARED ${CLIENT} ${CLIENT_TYPES_SRCS})
	# tell it to go to strategies folder
	set_target_properties(${FILENAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clients)
	target_include_directories(${FILENAME} PRIVATE src/data src/api lib/cpp-httplib/include lib/)
	target_link_libraries(${FILENAME} PRIVATE nlohmann_json OpenSSL::SSL OpenSSL::Crypto)
endforeach()

# copying over the website source
file(READ src/interface/webinterface.html WEBINTERFACE_HTML)
file(WRITE src/interface/webinterface.inc "R\"=====(${WEBINTERFACE_HTML})=====\"")

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
