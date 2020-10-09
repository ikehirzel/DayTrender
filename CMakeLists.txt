cmake_minimum_required(VERSION 3.9)
project(DayTrender)

file(GLOB DAYTRENDER_SRCS ./src/*.cpp ./src/api/*.cpp ./src/data/*.cpp ./src/util/*.cpp)

add_executable(daytrender ${DAYTRENDER_SRCS})
add_executable(dtbuild ./src/dtbuild/dtbuild.cpp ./src/util/fileutil.cpp ./src/util/strutil.cpp)

set_target_properties(daytrender PROPERTIES CXX_STANDARD 17)
set_target_properties(dtbuild PROPERTIES CXX_STANDARD 17)

set(HLOG_LIB_DIR "lib/hlog/lib")
file(COPY res DESTINATION .)

if(UNIX)
	target_link_libraries(daytrender PRIVATE pthread dl)
	file(COPY ${HLOG_LIB_DIR}/libhlog.so DESTINATION .)
	target_link_options(daytrender PRIVATE "-Wl,-rpath=.")
endif()

# Finding libraries
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Setting includes
target_include_directories(daytrender PRIVATE lib/cpp-httplib/include)
target_include_directories(daytrender PRIVATE lib/tinyplug/include)
target_include_directories(daytrender PRIVATE lib/hlog/include)

# Setting link directories
target_link_directories(daytrender PRIVATE lib/hlog/lib)

# Linking libraries
target_link_libraries(daytrender PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(daytrender PRIVATE nlohmann_json)
target_link_libraries(daytrender PRIVATE hlog)
