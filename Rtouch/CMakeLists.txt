cmake_minimum_required(VERSION 3.0.0)

project(project_Rtouch VERSION 0.1.0)

file(GLOB SOURCES "./src/*.cpp"     "../Log/src/Log.cpp"    "../threadPool/src/threadPool.cpp")
file(GLOB HEADERS "./include/*.h"   "../Log/include/Log.h"  "../threadPool/include/threadPool.hpp")

include_directories(include ../Log/include)
include_directories(include ../threadPool/include)
include_directories(include ./include)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} pthread)
