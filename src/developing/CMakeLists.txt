cmake_minimum_required (VERSION 2.6)
project (developing)

set (Developing_VERSION_MAJOR 1)
set (Developing_VERSION_MINOR 0)
set( CMAKE_CXX_FLAGS "-std=c++11 -O3")

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:") 
message(STATUS "    version: ${OpenCV_VERSION}") 
message(STATUS "    libraries: ${OpenCV_LIBS}") 
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

include_directories(${OpenCV_INCLUDE_DIRS})
 
include_directories("../../lib")

file(GLOB native_srcs "/home/zxb/SRC_C/socket_connect/lib/*.cpp")
add_library(
		test-lib
		SHARED
		${native_srcs}
)

# add the executable
add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server test-lib ${OpenCV_LIBS})
target_link_libraries(client test-lib ${OpenCV_LIBS})
