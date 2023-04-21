cmake_minimum_required(VERSION 3.0.0)

project(ThreadPool VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${CMAKE_BINARY_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../../out/" CACHE STRING "The path to use for make install" FORCE)
endif()

file(GLOB SOURCES "./src/*.cpp")
file(GLOB HEADERS "./include/*.hpp" "./include/*.h")

# 
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ../out/include
    ../out/lib/
)
link_directories(../out/lib/)

#
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
#
add_executable(Demo ${SOURCES} ${HEADERS} "./Demo.cpp")

# Copy header files to INSTALL path
install(FILES ${HEADERS} DESTINATION include)
# Copy library file to INSTALL path
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

#
target_link_libraries(${PROJECT_NAME} Log)
target_link_libraries(Demo Log pthread)
