cmake_minimum_required(VERSION 3.5.1)
project (concurency8)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
    message(FATAL "Require at least gcc-5.1")
endif()


set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR})
set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

AUX_SOURCE_DIRECTORY(./ PROJECT_SOURCE_DIR)
 
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")
add_executable( ${PROJECT_NAME}  ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} stdc++fs)
