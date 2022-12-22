cmake_minimum_required(VERSION 3.0.0)

project(project_server VERSION 0.1.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${CMAKE_BINARY_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../out/" CACHE STRING "The path to use for make install" FORCE)
endif()

file(GLOB SOURCES "./src/*.cpp"     "../Log/src/Log.cpp"    "../threadPool/src/threadPool.cpp")
file(GLOB HEADERS "./include/*.h"   "../Log/include/Log.h"  "../threadPool/include/threadPool.h")

include_directories(include ../Log/include/)
include_directories(include ../threadPool/include/)
include_directories(include ./include)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

target_link_libraries(${PROJECT_NAME} pthread)
