#---------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
#---------------------------------------------------------------------------------


#---------------------------------------------------------------------------------
# Setup the top most project
#---------------------------------------------------------------------------------
set(PROJECT_NAME "tlsexample")
message(STATUS "************  Starting ${PROJECT_NAME}  ************")
project(${PROJECT_NAME})

set(CMAKE_SUPPRESS_REGENERATION "1")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG -D_DEBUG")

file(GLOB HDRS "*.h" )
file(GLOB SRCS "*.c" "*.cpp" )

add_executable (${PROJECT_NAME} ${SRCS} ${HDRS})

target_link_libraries (${PROJECT_NAME} ssl crypto tls z )
