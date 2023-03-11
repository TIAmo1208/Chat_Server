cmake_minimum_required(VERSION 3.0.0)

project(Mysql VERSION 0.1.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Setting default CMAKE_INSTALL_PREFIX path to ${CMAKE_BINARY_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../../out/" CACHE STRING "The path to use for make install" FORCE)
endif()

# 
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(/usr/include/mysql)
# link_directories(/usr/lib/x86_64-linux-gnu)

# file(GLOB SOURCES "./src/*.cpp" "./Demo.cpp")
file(GLOB SOURCES "./src/*.cpp")
file(GLOB HEADERS "./include/*.hpp" "./include/*.h")

#
# add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

#
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
# Copy header files to INSTALL path
install(FILES ${HEADERS} DESTINATION include)
# Copy library file to INSTALL path
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)

# link the mysql lib
target_link_libraries(${PROJECT_NAME} mysqlclient pthread m rt atomic ssl crypto dl)
