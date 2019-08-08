cmake_minimum_required (VERSION 2.6)
project (developing)
add_definitions(-Linux)
set (Developing_VERSION_MAJOR 1)
set (Developing_VERSION_MINOR 0)
set( CMAKE_CXX_FLAGS "-std=c++11 -O3")

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:") 
message(STATUS "    version: ${OpenCV_VERSION}") 
message(STATUS "    libraries: ${OpenCV_LIBS}") 
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

include_directories(${OpenCV_INCLUDE_DIRS})
 
include_directories("./lib")
include_directories("./src/developing")
include_directories("./src/client")
include_directories("./lib/src/server")
include_directories("./lib/src/relay_server")

file(GLOB native_srcs "./lib/*.cpp")
add_library(
		test-lib
		SHARED
		${native_srcs}
		./src/client/client.cpp
		./src/server/server.cpp
		./src/relay_server/relay_server.cpp
)

# add the executable
add_executable(test_server ./src/developing/server.cpp)
add_executable(test_client ./src/developing/client.cpp)
add_executable(client ./src/client/main.cpp)
add_executable(server ./src/server/main.cpp)
add_executable(relay_server ./src/relay_server/main.cpp)


target_link_libraries(test_server test-lib ${OpenCV_LIBS})
target_link_libraries(test_client test-lib ${OpenCV_LIBS})
target_link_libraries(client test-lib ${OpenCV_LIBS})
target_link_libraries(server test-lib ${OpenCV_LIBS})
target_link_libraries(relay_server test-lib ${OpenCV_LIBS})


