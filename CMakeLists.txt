cmake_minimum_required(VERSION 3.5.1)
project (Concurency)

ADD_DEFINITIONS(-std=c++11 -pthread)

find_package (Threads)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/../${PROJECT_NAME}/build)
 
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
 
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
 
include_directories("${PROJECT_INCLUDE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")
 
add_subdirectory(concurency1)
add_subdirectory(concurency2)
add_subdirectory(concurency3)
add_subdirectory(concurency4)
add_subdirectory(concurency5)
add_subdirectory(concurency6)
add_subdirectory(concurency7)
add_subdirectory(concurency8)
add_subdirectory(concurency9)
add_subdirectory(concurency10)

