cmake_minimum_required(VERSION 3.14)
project("DayTrender")

# making sure it searches for dynamic libraries in same folder as executalble
set(CMAKE_INSTALL_RPATH "\$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH true)

# globbing sources for daytrender
file(GLOB DAYTRENDER_SRCS "src/main.cpp" "src/api/*.cpp" "src/data/*.cpp" "src/util/*.cpp")
file(GLOB STRATEGY_TYPES_SRCS src/data/strategydata.cpp src/data/indicator.cpp src/data/candle.cpp)
file(GLOB CLIENT_TYPES_SRCS
	"src/data/position.cpp"
	"src/data/account.cpp"
	"src/data/candle.cpp"
	"src/data/pricehistory.cpp"
)

# creating symlinks so files can be shared between build folder and project folder
file(CREATE_LINK ../config config SYMBOLIC)
file(CREATE_LINK ../strategies strategies SYMBOLIC)
file(CREATE_LINK ../clients clients SYMBOLIC)
file(CREATE_LINK ../src/interface/webinterface.html webinterface.html SYMBOLIC)


################################################################################
#		SETTING DAYTRENDER PROPERTIES
################################################################################

# creating main executable of project
add_executable(daytrender ${DAYTRENDER_SRCS})
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

# setting properties
set_target_properties(daytrender PROPERTIES CXX_STANDARD 17)

# finding required packages
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# linking daytrender libraries
target_link_libraries(daytrender PRIVATE ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS} OpenSSL::SSL OpenSSL::Crypto)

# setting include dirs
target_include_directories(daytrender PRIVATE
	"lib/cpp-httplib"
	"lib/cxx-logger/include"
	"lib/cxx-utils/include"
	"lib/cxx-plugin/include"
	"include"
)

################################################################################
#		COMPILING TESTS
################################################################################

# getting test sources
file(GLOB TEST_SRCS "src/test/*.cpp")

# loop through tests
foreach(TEST ${TEST_SRCS})
	# making executable for test
	get_filename_component(FILENAME ${TEST} NAME_WE)
	add_executable(${FILENAME}_test ${TEST})
	target_include_directories(${FILENAME}_test PRIVATE "include")
endforeach()

# stops cmake from prepending lib before plugin names
set(CMAKE_SHARED_LIBRARY_PREFIX "")

################################################################################
#		COMPILING STRATEGIES
################################################################################

# getting strategy sources
file(GLOB STRAT_SRCS "ext/strategies/*.cpp")

# loop through tests
foreach(STRAT ${STRAT_SRCS})
	# get filename without folder or extension
	get_filename_component(FILENAME ${STRAT} NAME_WE)
	# create shared object for it
	add_library(${FILENAME} SHARED ${STRAT} ${STRATEGY_TYPES_SRCS})
	# tell it to go to strategies folder
	set_target_properties(${FILENAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/strategies)
	target_include_directories(${FILENAME} PRIVATE "include")
endforeach()

################################################################################
#		COMPILING CLIENTS
################################################################################

# getting client sources
file(GLOB CLIENT_SRCS "ext/clients/*.cpp")

# loop through clients
foreach(CLIENT ${CLIENT_SRCS})
	# get filename without folder or extension
	get_filename_component(FILENAME ${CLIENT} NAME_WE)
	# create shared object for it
	add_library(${FILENAME} SHARED ${CLIENT} ${CLIENT_TYPES_SRCS})
	# tell it to go to strategies folder
	set_target_properties(${FILENAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/clients)
	target_include_directories(${FILENAME} PRIVATE
		"include"
		"lib/cpp-httplib"
		"lib/cxx-logger/include"
	)
	target_link_libraries(${FILENAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endforeach()

################################################################################
#		HANDLING WEB INTERFACE
################################################################################

file(READ src/interface/webinterface.html WEBINTERFACE_HTML)
file(WRITE src/interface/webinterface.inc "R\"=====(${WEBINTERFACE_HTML})=====\"")
